
#include <unistd.h>

#include "defs.h"

int comm_dev_send(struct context *p)
{
    uint8_t *raw;
    int len = p->packet.len;

    /* add checksum */
    packet_putc(&p->packet, 0); /* add place holder */

    raw = packet_raw(&p->packet);
    len = packet_raw_len(&p->packet);

    raw[len-1] = packet_checksum(& p->packet); /* replace placeholder */

    if(p->debug)
        util_hexdump(">> sending...", raw, len);

    return buffer_write(&p->buffer, raw, len);
}

int comm_dev_recv(struct context *p)
{
    uint8_t c;
    int ret = packet_read(& p->packet, &p->buffer);

    /* DEBUG */
    if(p->debug) {
        util_hexdump("<< received...",
                     packet_raw(&p->packet),
                     packet_raw_len(&p->packet));
        printf("\n");
    }


    /* see if checksum is good */
    c = packet_checksum(& p->packet);

    if(ret) {
        if(c == 0) {
            p->packet.len--; /* remove checksum byte itself */
            return 1;
        }
        printf("Checksum failure: %02x\n", c);
    }
    return 0;
}


int comm_file_recv(struct context *p)
{
    return packet_fread(& p->packet, p->fp) > -1;
}


int comm_dev_sync(struct context *p)
{
    int i, state;
    char *din = ".......\r\n";
    uint8_t *dout = "$\x05SYNC\x9a";
    uint8_t c;

    if(!buffer_write(&p->buffer, din, strlen(din))) {
        fprintf(stderr, "sync failed: could not write sync message\n");
        return 0;
    }

    for(state = 0;;) {
        if(!buffer_read(&p->buffer, &c)) {
            fprintf(stderr, "sync failed: could not read sync message\n");
            return 0;
        }

        if(c  == dout[state]) {
            state ++;
            if(!dout[state]) {
                return 1;
            }
        } else {
            state = 0;
        }
    }
    return 0;
}
