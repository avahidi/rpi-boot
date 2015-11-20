#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


struct buffer {
#define BUFFER_SIZE 128
    uint8_t buffer[BUFFER_SIZE];
    int ptr, has;
    int fd;
};

extern int buffer_read(struct buffer *b, uint8_t *c);
extern int buffer_write(struct buffer *b, uint8_t *data, int len);
extern void buffer_init(struct buffer *n, int fd);


enum reg { REG_R0, REG_R1, REG_PSR, REG_PC, REG_ADR };

struct packet {
#define PACKET_DATA_MAX 64
    uint8_t pad[2];
    uint8_t cmd;
    int8_t len;
    uint8_t data[PACKET_DATA_MAX + 2];
} __packed;

extern void util_hexdump(char *msg, char *buffer, int len);
extern void util_recode(char *str, int *len);

extern uint8_t *packet_raw(struct packet *b);
extern int packet_raw_len(struct packet *b);

extern void packet_reset(struct packet *b, uint8_t cmd);
extern int packet_putc(struct packet *b, int c);
extern int packet_putci(struct packet *b, char *s, int cnt);
extern int packet_puti(struct packet *b, uint32_t i);

extern uint8_t packet_checksum(struct packet *b);

extern int packet_fread(struct packet *p, FILE *fp);
extern int packet_read(struct packet *p, struct buffer *b);

struct context {
    /* input */
    char *inname;
    FILE *fp; /* file or stdin */

    /* device */
    char *outname;
    int fd; /* device */
    fd_set rfds;

    uint32_t adr;

    /* flags */
    int debug;
    int verify;
    int dontrun;

#define OPTSTART_WDOG 1
#define OPTSTART_SEC  2
#define OPTSTART_HYP  4
    uint8_t optstart;

    struct packet packet;
    struct buffer buffer;
};

extern int comm_dev_sync(struct context *p);
extern int comm_dev_send(struct context *p);
extern int comm_dev_recv(struct context *p);
extern int comm_file_recv(struct context *p);

extern int process_file(struct context *p);
extern int process_interactive(struct context *p);

#endif /* __DEFS_H__ */
