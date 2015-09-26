
#include "defs.h"
#include <time.h>

int cmd_send(struct context *p) 
{        
    int i;
    struct context copy = *p;
          
    for(i = 0; i < 8; i ++) {
        *p = copy;
        
        if(!comm_dev_send(p)) {
            fprintf(stderr, "could not send data\n");
            return 0;
        }        
        
        if(!comm_dev_recv(p)) {
            fprintf(stderr, "could not receive response\n");
            return 0;
        }
        
        if(p->buffer.len >= 1 && p->buffer.cmd == '!' && p->buffer.data[0] == 'f') {
            printf("Failed to send data, will try again (%d)...\n", i);
            // nanosleep(50000);
        } else {
            break;
        }
    }
    
    if(p->buffer.cmd == '$')
        return 1;
    
    fprintf(stderr, "Failed or bad response: %c\n", p->buffer.data[0]);
    return 0;
}

char *cmd_id(struct context *p)
{
    struct buffer *b = &p->buffer;            
    buffer_reset(b, '?');
    return cmd_send(p) ? & b->data[0] : 0;
}

int cmd_start(struct context *p)
{
    struct buffer *b = &p->buffer;            
    buffer_reset(b, 's');
    return cmd_send(p);
}

int cmd_set_reg(struct context *p, enum reg reg, uint32_t data)
{
    struct buffer *b = &p->buffer;        
    buffer_reset(b, 'g');;
    buffer_putc(b, reg);
    buffer_puti(b, data);    
    return cmd_send(p);
}

int cmd_get_reg(struct context *p, enum reg reg)
{    
    struct buffer *b = &p->buffer;    
    buffer_reset(b, 'g');
    buffer_putc(b, reg);
    return cmd_send(p);
}

int cmd_set_adr(struct context *p, uint32_t adr)
{
    cmd_set_reg(p, REG_ADR, adr);
}

int cmd_write(struct context *p, char *data, int len)
{
    struct buffer *b = &p->buffer;
    buffer_reset(b, 'w');
    buffer_putci(b, data, len);
    return cmd_send(p);    
}

int cmd_read(struct context *p, char *data, int len)
{
    struct buffer *b = &p->buffer;    
    int ret;
    
    buffer_reset(b, 'r');
    buffer_putc(b, len);
    
    ret = cmd_send(p);
    if(ret && b->len == len) {
        memcpy(data, b->data, b->len);
        return 1;
    }
    
    return 0;
}

void process_file(struct context *p)
{
#define COPY_LENGTH 8    
    char bufferw[COPY_LENGTH];    
    char bufferr[COPY_LENGTH];            
    
    uint32_t adr0, adr;
    int i, n;
        
    printf("Connected to '%s'...\n", cmd_id(p));
    
    
    cmd_set_adr(p, adr0 = adr = p->adr);

    while(!feof(p->fp)) {
        
        n = fread(bufferw, 1, COPY_LENGTH, p->fp);
        if(n <= 0) break;                        
        if(!cmd_write(p, bufferw, n))
            return;
        
        /* verify it */
        if(!cmd_set_adr(p, adr)) {
            fprintf(stderr, "set_adr failed\n");
            return;        
        }
        if(!cmd_read(p, bufferr, n)) {
            fprintf(stderr, "read failed\n");            
            return;        
        }
        if(!memcpy(bufferw, bufferr + 1, n)) {
            fprintf(stderr, "readback failed\n");
            return;
        }
                              
        printf(" %08x   \r", adr); 
        fflush(stdout);
        adr += n;        
    }
    
    printf("\n");
    
    printf("Starting at 0x%08x...", adr0);
    cmd_set_reg(p, REG_PC, adr0);
    printf("%s\n", cmd_start(p) ? "DONE" : "FAILED");
    
    
    
}

void process_interactive(struct context *p)
{    
    int n;    
    p->debug = 1;
    
    for(;;) {
        fprintf(stdout, "> ");
        fflush(stdout);
        
        comm_file_recv(p);        
        
        if(p->buffer.cmd == 'q' && p->buffer.len == 0)
            return;
        
        n = p->buffer.len;
        util_recode(p->buffer.data, &n);
        p->buffer.len = n;
        
        
        if(p->buffer.len > -1) {
            cmd_send(p);
            
            p->buffer.data[p->buffer.len] = '\0';            
            printf(": %s\n", p->buffer.data);

        }
    }

}
