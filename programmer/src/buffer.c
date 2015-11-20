
#include "defs.h"

#include <errno.h>
#include <unistd.h>

int buffer_write(struct buffer *b, uint8_t *data, int len)
{
    int n;
    while(len > 0) {
        n = write(b->fd, data, len);
        if(n < 0) {
            perror("write failed");
            return 0;
        } else if(n == 0) {
            fprintf(stderr, "ERROR: write failed\n");
            return 0; /* timeout */
        }
        len -= n;
        data += len;
    }
    return 1;
}

int buffer_read(struct buffer *b, uint8_t *c)
{
    fd_set fds;
    struct timeval timeout;
    int n;

    if(b->ptr >= b->has) {
        /* wait for data */
        FD_ZERO(&fds);
        FD_SET(b->fd, &fds);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        n = select(b->fd + 1, &fds, NULL, NULL, &timeout);
        if(n < 0) {
            perror("Read failed");
            return 0;
        } else if(n == 0) {
            fprintf(stderr, "ERROR: read timeout\n");
            return 0; /* timeout */
        }

        n = read(b->fd, b->buffer, BUFFER_SIZE);
        if(n < 0) { /* shoud not happen ... */
            perror("Could not read from fd");
            return 0;
        }

        b->ptr = 0;
        b->has = n;
    }
    *c = 0xFF & b->buffer[b->ptr++];
    return 1;
}


void buffer_init(struct buffer *b, int fd)
{
    b->fd = fd;
    b->ptr = 0;
    b->has = 0;
}
