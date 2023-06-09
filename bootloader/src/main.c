
#include "defs.h"

#define PROTO_SIZE_MAX 32

#define PROTO_ERR_NONE '!'
#define PROTO_ERR_PARAM 'p'
#define PROTO_ERR_UNKNOWN 'u'
#define PROTO_ERR_FORMAT 'f'
#define PROTO_ERR_SIZE 's'

struct wdog {
    uint32_t status;
    uint32_t rstc;
    uint32_t rsts;
    uint32_t wdog;
};


#define UART_CLOCK  3000000
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

    uint32_t ifls;
    uint32_t imsc;
    uint32_t ris;
    uint32_t mis;
    uint32_t icr;
    /* ... */
};

struct proto {
    /* input */
    uint8_t data[PROTO_SIZE_MAX];
    int len;
    uint8_t cmd;
    uint8_t sum;
    uint8_t cnt;

    /* state */
    union {
        uint32_t regs[5]; /* R0, R1, PSR, PC, ptr */
        struct {
            uint32_t regs_[4];
            uint32_t *ptr;
        } __packed;
    };
};


struct gpio {
    uint32_t sel[6];
    uint32_t unused0;
    uint32_t set[2];
    uint32_t unused1;
    uint32_t clr[2];
    /* ... */
};

enum gpio_sel {
    GPIO_IN = 0,
    GPIO_OUT,
    GPIO_ALT5,
    GPIO_ALT4,
    GPIO_ALT0,
    GPIO_ALT1,
    GPIO_ALT2,
    GPIO_ALT3
};

typedef void (*proto_callback)(struct proto *p);

struct proto_handler {
    uint8_t cmd;
    int8_t min_size, max_size;
    proto_callback callback;
};

uint8_t uart_sum = 0;
uint32_t cpu0_psr = 0x1D3;

/* forward references */
void uart_write(int c);

/*
 * GPIO
 */

void gpio_sel(int pin, enum gpio_sel func)
{
    volatile struct gpio *_gpio = (struct gpio *) GPIO_BASE;
    const int index = pin / 10;
    const int shift = (pin - index * 10) * 3;
    _gpio->sel[index] = (func << shift) | (_gpio->sel[index] & ~(7 << shift));
}

void gpio_set(int pin, int val)
{
    volatile struct gpio *_gpio = (struct gpio *) GPIO_BASE;
    const int index = pin >> 5;
    const int shift = pin & 31;

    if(val)
        _gpio->set[index] = 1UL << shift;
    else
        _gpio->clr[index] = 1UL << shift;
}


void jtag_init()
{
    /* configure JTAG pins */
#if 1
    /* upper section of the GPIO connector */
    gpio_sel(22, GPIO_ALT4); /* 22 -> TRST */
    gpio_sel(4 , GPIO_ALT5); /*  4 -> TDI  */
    gpio_sel(27, GPIO_ALT4); /* 27 -> TMS  */
    gpio_sel(25, GPIO_ALT4); /* 25 -> TCK  */
    gpio_sel(24, GPIO_ALT4); /* 24 -> TDO  */
#else
    /* mostly the lower section of the GPIO connector */
    gpio_sel(22, GPIO_ALT4); /* 22 -> TRST */
    gpio_sel(26, GPIO_ALT4); /* 26 -> TDI  */
    gpio_sel(5 , GPIO_ALT5); /*  5 -> TDO  */
    gpio_sel(6 , GPIO_ALT5); /*  6 -> RTCK */
    gpio_sel(12, GPIO_ALT5); /* 12 -> TMS  */
    gpio_sel(13, GPIO_ALT5); /* 13 -> TCK  */
#endif
}


void debug_init()
{
    int i;
    uint32_t tmp;

    /* wiggle the LED to show we are allive */
#define GPIO_LED 47
    gpio_sel(GPIO_LED, GPIO_OUT);
    for(i = 16; i ; --i) {
        /* simple delay */
        tmp = 1UL << 18;
        while(--tmp) __asm__ volatile("nop");

        /* toggle LED */
        gpio_set(GPIO_LED, i & 1);

        /* say something */
        uart_write(i & 1 ?  '-' : '_');

    }
    gpio_sel(GPIO_LED, GPIO_IN); /* disable led output */
    uart_write('\r'); /* end line */
    uart_write('\n');

}

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
    /* no interrupts */
    _pl011->imsc = 0;
    _pl011->icr = -1;

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
 * watchdog
 */

void wdog_enable()
{
    volatile struct wdog * _wdog = (struct wdog *)WDOG_BASE;
    _wdog->wdog = 0x5A0FFFFF; /* 16 secs */
    _wdog->rstc = (_wdog->rstc & ~0x30) | 0x5a000020; /* enable it */
}

/*
 * PROTOCOL
 */

/* read32/write32 ensure 32-bit access in case target is iomem */
void proto_mem_read32(struct proto *p, uint8_t *buffer)
{
    uint32_t tmp = *p->ptr++;
    buffer[3] = (tmp >> 24) & 0xFF;
    buffer[2] = (tmp >> 16) & 0xFF;
    buffer[1] = (tmp >> 8 ) & 0xFF;
    buffer[0] = (tmp >> 0 ) & 0xFF;
}

void proto_mem_write32(struct proto *p, uint8_t *buffer)
{
    *p->ptr++ = (buffer[1] << 8) | buffer[0] |
          (buffer[3] << 24) | (buffer[2] << 16);
}


/* read/write packet including size and checksum handling */

void proto_sync(struct proto *p); /* forward */

int proto_read(struct proto *p)
{
    int i;
    uint8_t sum;

    p->cnt ++;

    do {
        p->cmd = uart_read();

        if(p->cmd == '.')
            proto_sync(p);
    } while(p->cmd == '.' || p->cmd == '\r' || p->cmd == '\n');

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

    p->sum = sum; // for debugging

    /* remove & check checksum */
    p->len--;
    return sum == 0;
}

void proto_write(int okay, const uint8_t *data, int data_len)
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
    uint8_t err[5];
    err[0] = type;
    err[1] = p->cmd;
    err[2] = p->len;
    err[3] = p->sum;
    err[4] = p->cnt;
    proto_write(0, err, sizeof(err));
}

void proto_okay(struct proto *p)
{
    proto_write(1, 0, 0);
}

void proto_sync(struct proto *p)
{
    char c;

    for(;;) {
        c = uart_read();
        if(c == '\n' || c == '\r') break;
    }

    proto_write(1, (uint8_t *)"SYNC", 4);
}

/* the handlers */
void proto_handle_about(struct proto *p)
{
    proto_write(1, (uint8_t *)"minipi bootld.", 14);
}

void proto_handle_write(struct proto *p)
{
    int i;
    for(i = 0; i < p->len; i += 4)
        proto_mem_write32(p, p->data + i);
    proto_okay(p);
}

void proto_handle_read(struct proto *p)
{
    int i, len = p->data[0];
    for(i = 0; i < len; i += 4)
        proto_mem_read32(p, p->data + i);
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

    proto_write(1, (uint8_t *)&p->regs[r], sizeof(uint32_t));
}

void __naked proto_handle_start(struct proto *p)
{
    /* response first since we are not going to return */
    proto_okay(p);

    /* BIT0: enable watchdog ? */
    if(p->data[0] & 0x01)
        wdog_enable();

    /* BIT2: switch to hyp mode*/
    if(p->data[0] & 0x04)
        p->regs[2] = 0x1DA; /* SPSR = hyp mode. I, F & A set */

    /* BIT1 switch to secure mode */
    else if(p->data[0] & 0x02)
        __smc(0, 1);

    /* save this for other cores */
    cpu0_psr = p->regs[2];

    /* change mode and start it */
    __asm__ volatile(
                     /* save p->regs */
                     "mov r12, %0\n"

                     /* change mode to the given CPSR */
                     "mov r0, #1\n"
                     "ldr r1, [r12, #8]\n"
                     "smc #0\n"

                     /* get R0, R1 and LR and jump to LR */
                     "ldm r12, {r0-r2, lr}\n"
                     "mov pc, lr\n"
                     :: "r"(&p->regs[0]));
}

struct proto_handler handlers[] = {
    {'?', 0, 0, proto_handle_about },
    {'w', 1, -1, proto_handle_write},
    {'r', 1, -1, proto_handle_read},
    {'g', 1, 5, proto_handle_reg},
    {'s', 1, 1, proto_handle_start},
    {0, 0, 0, 0 }
};

void proto_handle(struct proto *p)
{
    int len, cmd;
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
    uint32_t tmp;

    /* init */
    jtag_init();
    uart_init();
    debug_init();

    /* get some default values */
    __asm__ volatile("mrs %0, cpsr" : "=r"(tmp));
    p.regs[0] = 0xcafebabe;
    p.regs[1] = 0xdeadbeef;
    p.regs[2] = tmp;
    p.regs[3] = RAM_START + RAM_SIZE;
    p.ptr = (uint32_t *) p.regs[3];

    /* start the command-response loop */
    for(;;) {
        if(proto_read(&p))
            proto_handle(&p);
        else
            proto_error(&p, PROTO_ERR_FORMAT);
    }
}
