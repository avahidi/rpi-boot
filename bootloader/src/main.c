
#include "defs.h"

#define PROTO_SIZE_MAX 32

#define PROTO_ERR_NONE '!'
#define PROTO_ERR_PARAM 'p'
#define PROTO_ERR_UNKNOWN 'u'
#define PROTO_ERR_FORMAT 'f'
#define PROTO_ERR_SIZE 's'


#define UART_CLOCK 3000000
#define UART_RATE 115200

struct pl011 {
    uint32_t dr;
    uint32_t rsr;
    uint32_t reserved0[4];    
    uint32_t fr;
    uint32_t reserved1;
    uint32_t ilpr;
    uint32_t ibrd;
    uint32_t fbrd;
    uint32_t lcr;
    uint32_t cr;
    /* ... */    
};

struct proto {    
    /* input */    
    uint8_t data[PROTO_SIZE_MAX];
    int len;        
    char cmd;
    
    /* state */
    union {
        uint32_t regs[5]; /* R0, R1, PSR, PC, ptr */
        struct {
            uint32_t regs_[4];
            uint8_t *ptr;
        } __packed;
    };
};

typedef void (*proto_callback)(struct proto *p);

struct proto_handler {
    uint8_t cmd;
    int8_t min_size, max_size;
    proto_callback callback;    
};

uint8_t uart_sum = 0;

/*
 * UART
 */

void uart_init()
{
    volatile struct pl011 * _pl011 = (struct pl011 *) UART_BASE;
    uint32_t div;
    
    _pl011->cr = 0; /* disable */    
    div = (UART_CLOCK * 4) /  UART_RATE;
    _pl011->ibrd = div >> 6;
    _pl011->fbrd = div & 63;        
    _pl011->lcr = 0x0070; /* 8bit, 1 stop, no parities */
    _pl011->cr = 0x0301; /* enable */
}

void uart_write(int c)
{
    volatile struct pl011 * _pl011 = (struct pl011 *) UART_BASE;    
        
    while( _pl011->fr & (1UL << 5))
        /* wait for non-full FIFO */ ;
    
    _pl011->dr = c;        
    uart_sum += c;
}

int uart_read()
{
    volatile struct pl011 * _pl011 = (struct pl011 *) UART_BASE;
    while( _pl011->fr & (1UL << 4))
        /* wait for recv FIFO to become non-empty */ ;
        
    return _pl011->dr;    
}

/*
 * PROTOCOL
 */

int proto_read(struct proto *p)
{
    int i, c;
    uint8_t sum;
    
    p->cmd = uart_read();
    p->len = uart_read(); /* including checksum */
                       
    /* too large ? */
    if(p->len >= PROTO_SIZE_MAX) {
        p->len = 0;
        return 0;          
    }
    
    
    /* read additional data - while calcucalting checksum */    
    sum = p->cmd + p->len;
    for(i = 0; i < p->len; i++)  {
        p->data[i] = uart_read();
        sum += p->data[i];        
    }
        
    
    /* remove & check checksum */
    p->len--;
    return sum == 0;
}

void proto_write(int okay, uint8_t *data, int data_len)
{
    int i;    
    uart_sum = 0;
    uart_write(okay ? '$' : '!');
    uart_write(data_len + 1);    
    for(i = 0; i < data_len; i ++)
        uart_write(data[i]);
    uart_write(-uart_sum); /* "checksum" */
}

void proto_error(struct proto *p, int type)
{
    char t = type;
    proto_write(0, & t, 1);
}

void proto_okay(struct proto *p)
{
    proto_write(1, 0, 0);
}

/* the handlers */
void proto_handle_about(struct proto *p)
{    
    proto_write(1, "minipi bootld.", 14);
}

void proto_handle_write(struct proto *p)
{
    int i;
    for(i = 0; i < p->len; i++)
        *p->ptr++ = p->data[i];
    
    proto_okay(p);    
}

void proto_handle_read(struct proto *p)
{
    int i, len = p->data[0];    
    for(i = 0; i < len; i++)
        p->data[i] = *p->ptr++;
    
    proto_write(1, &p->data[0], len);   
}

void proto_handle_reg(struct proto *p)
{    
    int i, r;
    uint32_t data;
    
    /* what register ? */
    r = p->data[0];
    if(r < 0 || r >= 5) {
        proto_error(p, PROTO_ERR_PARAM);
        return;
    }
    
    /* if available, read input and write to selected register */
    if(p->len > 1) {
        for(data = 0, i = 1; i < p->len; i++)
            data = (data << 8) | p->data[i];
        p->regs[r] = data;
    }

    // proto_okay(p);
    proto_write(1, (uint8_t *)& p->regs[r], 4); // DEBUG
}

void proto_handle_echo(struct proto *p)
{    
    proto_write(1, p->data, p->len);
}

void proto_handle_start(struct proto *p)
{
    /* response first since we are not going to return */
    proto_okay(p);
    
    /* read guest registers and start it */
    __asm__ volatile(
                     "mov r10, %0\n"
                     "ldm r10, {r0-r2, lr}\n"
                     "msr spsr, r2\n"
                     "movs pc, lr\n"
                     :: "r"(& p->regs[0]));
}

struct proto_handler handlers[] = {
    {'?', 0, 0, proto_handle_about },
    {'w', 1, -1, proto_handle_write},
    {'r', 1, -1, proto_handle_read},
    {'g', 1, 5, proto_handle_reg},
    {'s', 0, 0, proto_handle_start},    
    {'e', 0, -1, proto_handle_echo},    
    {0, 0, 0, 0 }
};

void proto_handle(struct proto *p)
{
    int i, len, cmd;
    struct proto_handler *h;    
    /* data length, cmd excluded */
    len = p->len;
    cmd = p->cmd;
    
    for(h = &handlers[0]; h->callback; h++) {
        if(h->cmd == cmd) {
            /* check length */
            if(len < h->min_size ||
               (len > h->max_size && h->max_size != -1)) {
                proto_error(p, PROTO_ERR_SIZE);                
                return;
            } else {
                /* all clear, call handler */
                h->callback(p);
                return;
            }
                
        }
    }

    /* if we are here, then the commend was unknown */
    proto_error(p, PROTO_ERR_UNKNOWN);
}

/*
 * main
 */

void main()
{
    struct proto p;
    int i, e;
    uint32_t tmp;
    
    /* init */
    uart_init();
    
    /* get some default values */
    __asm__ volatile("mrs %0, cpsr" : "=r"(tmp));
    p.regs[0] = 0xcafebabe;
    p.regs[1] = 0xdeadbeef;
    p.regs[2] = tmp;
    p.regs[3] = RAM_START + RAM_SIZE;    
    p.ptr = (uint8_t *) p.regs[3];

    /* start the command-response loop */
    for(;;) {
        if(proto_read(&p))
            proto_handle(&p);
        else
            proto_error(&p, PROTO_ERR_FORMAT);
    }    
}
