
#include "defs.h"

#include <errno.h>
#include <unistd.h>


/* buffered reading + some bug workarounds*/
static char helper_readchar(int fd)
{
#define READ_SIZE 128
    static int ptr = 0, has = 0;
    static char buffer[READ_SIZE];
    
    if(ptr >= has) {
        ptr = 0;    
        
        /* fd is non-blocking, but we don't own the device */
        do {
            for(;;) {
                has = read(fd, buffer, READ_SIZE);
                if(!(has == -1 && errno == EAGAIN))
                    break;
                nanosleep(1000); /* this is just plain stupid... */
            }
        } while(has < 1);                        
    }
    
    return 0xFF & buffer[ptr++];
}


void buffer_reset(struct buffer *b, uint8_t cmd)
{
    b->cmd = cmd;
    b->len = 0;
}

uint8_t *buffer_raw(struct buffer *b)
{
    return &b->data[-2];
}

int buffer_raw_len(struct buffer *b)
{
    return b->len + 2;
}


uint8_t buffer_checksum(struct buffer *b)
{    
    int i;
    uint8_t sum;
    uint8_t *raw = buffer_raw(b);
    int len = b->len + 2;
    
    for(i = 0, sum = 0; i < len; i++)
        sum += raw[i];
    
    return -sum;
}


int buffer_putc(struct buffer *b, int c)
{
    if(b->len >= BUFFER_SIZE) 
        return 0;
    
    b->data[b->len++] = c;
    return 1;
}

int buffer_putci(struct buffer *b, char *s, int cnt)
{
    if(b->len + cnt >= BUFFER_SIZE) 
        return 0;
    
    memcpy( & b->data[b->len], s, cnt);
    b->len += cnt;
    return 1;
}

int buffer_puti(struct buffer *b, uint32_t i)
{
    return 
          buffer_putc(b, 0xFF & (i >> 24)) && 
          buffer_putc(b, 0xFF & (i >> 16)) && 
          buffer_putc(b, 0xFF & (i >>  8)) && 
          buffer_putc(b, 0xFF & (i >> 0));

}


int buffer_fread(struct buffer *b, FILE *fp)
{
    char *tmp;
    char buffer[BUFFER_SIZE + 2];    
    int len;
    
    fgets(buffer, BUFFER_SIZE, fp);
    buffer[BUFFER_SIZE] = '\0';
    
    /* fgets includes line end */
    tmp = strchr(buffer, '\r');
    if(tmp) *tmp = '\0';
    tmp = strchr(buffer, '\n');
    if(tmp) *tmp = '\0';        
    
    len = strlen(buffer);
    if(len > 0) {
        buffer_reset(b, buffer[0]);
        buffer_putci(b, buffer + 1, len - 1);
        b->data[b->len] = '\0'; /* make it printable */
    } else {
        b->cmd = 0;
        b->len = -1;
    }
    
    return b->len;
}

int buffer_read(struct buffer *b, int fd)
{
    int i;
    
    b->cmd = helper_readchar(fd);
    b->len = helper_readchar(fd);
        
    if(b->len > BUFFER_SIZE) {
        fprintf(stderr, "buffer contains too much data (%d)\n", b->len);
        return 0;
    }    
    
    for(i = 0; i < b->len; i++) 
        b->data[i] = helper_readchar(fd);
    
    return 1;
}
