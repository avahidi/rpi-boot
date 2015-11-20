
#include "defs.h"

#include <errno.h>
#include <unistd.h>


/* buffered reading + timeout */
static int helper_readchar(int fd, uint8_t *c)
{
#define READ_SIZE 128
    static int ptr = 0, has = 0;
    static uint8_t buffer[READ_SIZE];
    struct timeval timeout;


    if(ptr >= has) {
        ptr = 0;

        /* wait for data */
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
#if 0
        if(select(p.fd + 1, &p.readfs, NULL, NULL, &timeout) != 1)
            return 0; /* timeout */
#endif
        has = read(fd, buffer, READ_SIZE);
        if(has == -1) /* shoud not happen ... */
            return 0;
    }

    *c = 0xFF & buffer[ptr++];
    return 1;
}


void packet_reset(struct packet *b, uint8_t cmd)
{
    b->cmd = cmd;
    b->len = 0;
}

uint8_t *packet_raw(struct packet *b)
{
    return &b->data[-2];
}

int packet_raw_len(struct packet *b)
{
    return b->len + 2;
}


uint8_t packet_checksum(struct packet *b)
{
    int i;
    uint8_t sum;
    uint8_t *raw = packet_raw(b);
    int len = b->len + 2;

    for(i = 0, sum = 0; i < len; i++)
        sum += raw[i];

    return -sum;
}


int packet_putc(struct packet *b, int c)
{
    if(b->len >= PACKET_DATA_MAX)
        return 0;

    b->data[b->len++] = c;
    return 1;
}

int packet_putci(struct packet *b, char *s, int cnt)
{
    if(b->len + cnt >= PACKET_DATA_MAX)
        return 0;

    memcpy( & b->data[b->len], s, cnt);
    b->len += cnt;
    return 1;
}

int packet_puti(struct packet *b, uint32_t i)
{
    return
          packet_putc(b, 0xFF & (i >> 24)) &&
          packet_putc(b, 0xFF & (i >> 16)) &&
          packet_putc(b, 0xFF & (i >>  8)) &&
          packet_putc(b, 0xFF & (i >> 0));

}


int packet_fread(struct packet *b, FILE *fp)
{
    char *tmp;
    char buffer[PACKET_DATA_MAX + 2];
    int len;

    fgets(buffer, PACKET_DATA_MAX, fp);
    buffer[PACKET_DATA_MAX] = '\0';

    /* fgets includes line end */
    tmp = strchr(buffer, '\r');
    if(tmp) *tmp = '\0';
    tmp = strchr(buffer, '\n');
    if(tmp) *tmp = '\0';

    len = strlen(buffer);
    if(len > 0) {
        packet_reset(b, buffer[0]);
        packet_putci(b, buffer + 1, len - 1);
        b->data[b->len] = '\0'; /* make it printable */
    } else {
        b->cmd = 0;
        b->len = -1;
    }

    return b->len;
}

int packet_read(struct packet *p, struct buffer *b)
{
    int i;

    if(!buffer_read(b, &p->cmd))
        return 0;

    if(!buffer_read(b, &p->len))
        return 0;


    if(p->len > PACKET_DATA_MAX) {
        fprintf(stderr, "packet contains too much data (%d)\n", p->len);
        return 0;
    }

    for(i = 0; i < p->len; i++)
        if(!buffer_read(b, &p->data[i]))
            return 0;

    return 1;
}
