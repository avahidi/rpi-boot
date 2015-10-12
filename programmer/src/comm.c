

#include "defs.h"

int comm_dev_send(struct context *p)
{
    uint8_t *raw;
    int len = p->buffer.len;

    /* add checksum */
    buffer_putc(&p->buffer, 0); /* add place holder */

    raw = buffer_raw(&p->buffer);
    len = buffer_raw_len(&p->buffer);

    raw[len-1] = buffer_checksum(& p->buffer); /* replace placeholder */

    if(p->debug)
        util_hexdump(">> sending...", raw, len);

    return write(p->fd, raw, len ) == len;
}

int comm_dev_recv(struct context *p)
{
    uint8_t c;
    int ret = buffer_read(& p->buffer, p->fd) > 0;

    /* DEBUG */
    if(p->debug) {
        util_hexdump("<< received...",
                     buffer_raw(&p->buffer),
                     buffer_raw_len(&p->buffer));
        printf("\n");
    }


    /* see if checksum is good */
    c = buffer_checksum(& p->buffer);

    if(ret) {
        if(c == 0) {
            p->buffer.len--; /* remove checksum byte itself */
            return 1;
        }

        printf("Checksum failure: %02x\n", c);
    }
    return 0;
}


int comm_file_recv(struct context *p)
{
    return buffer_fread(& p->buffer, p->fp) > -1;
}


int comm_dev_sync(struct context *p)
{
    int i, state;
    char *din = ".......................\r";
    char *dout = "$\05SYNC";
    char c;

    if(write(p->fd, din, strlen(din)) != strlen(din)) {
        fprintf(stderr, "sync failed: could not write sync message\n");
        return 0;
    }

    for(state = 0;;) {
        i = read(p->fd, &c, 1);
        if(i < 1) {
            fprintf(stderr, "sync failed: could not read sync message\n");
            return 0;
        }

        if(c  == dout[state]) {
            state ++;
            if(!dout[state]) {
                read(p->fd, &c, 1); /* pass on checksum for now */
                return 1;
            }
        } else {
            state = 0;
        }
    }
    return 0;
}
