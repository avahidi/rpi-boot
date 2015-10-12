#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 64

enum reg { REG_R0, REG_R1, REG_PSR, REG_PC, REG_ADR };

struct buffer {
    uint8_t pad[2];
    uint8_t cmd;
    int8_t len;
    uint8_t data[BUFFER_SIZE + 2];
} __packed;

extern void util_hexdump(char *msg, char *buffer, int len);
extern void util_recode(char *str, int *len);

extern uint8_t *buffer_raw(struct buffer *b);
extern int buffer_raw_len(struct buffer *b);

extern void buffer_reset(struct buffer *b, uint8_t cmd);
extern int buffer_putc(struct buffer *b, int c);
extern int buffer_putci(struct buffer *b, char *s, int cnt);

extern uint8_t buffer_checksum(struct buffer *b);

extern int buffer_fread(struct buffer *b, FILE *fp);
extern int buffer_read(struct buffer *b, int fd);

struct context {
    /* input */
    char *inname;
    FILE *fp; /* file or stdin */

    /* device */
    char *outname;
    int fd; /* device */

    uint32_t adr;

    /* flags */
    int debug;
    int verify;
    int dontrun;
    int watchdog;

    struct buffer buffer;
};

extern int comm_dev_sync(struct context *p);
extern int comm_dev_send(struct context *p);
extern int comm_dev_recv(struct context *p);
extern int comm_file_recv(struct context *p);

extern int process_file(struct context *p);
extern int process_interactive(struct context *p);

#endif /* __DEFS_H__ */
