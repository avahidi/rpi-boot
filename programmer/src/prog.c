
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

        if(p->packet.len >= 1 && p->packet.cmd == '!' && p->packet.data[0] == 'f') {
            printf("Failed to send data, will try again (%d)...\n", i);
            // nanosleep(50000);
        } else {
            break;
        }
    }

    if(p->packet.cmd == '$')
        return 1;

    fprintf(stderr, "Failed or bad response: %c\n", p->packet.data[0]);
    return 0;
}

char *cmd_id(struct context *p)
{
    struct packet *b = &p->packet;
    packet_reset(b, '?');
    return cmd_send(p) ? & b->data[0] : 0;
}

int cmd_start(struct context *p)
{
    struct packet *b = &p->packet;
    packet_reset(b, 's');
    packet_putc(b, p->optstart);
    return cmd_send(p);
}

int cmd_set_reg(struct context *p, enum reg reg, uint32_t data)
{
    struct packet *b = &p->packet;
    packet_reset(b, 'g');
    packet_putc(b, reg);
    packet_puti(b, data);
    return cmd_send(p);
}

int cmd_get_reg(struct context *p, enum reg reg)
{
    struct packet *b = &p->packet;
    packet_reset(b, 'g');
    packet_putc(b, reg);
    return cmd_send(p);
}

int cmd_set_adr(struct context *p, uint32_t adr)
{
    cmd_set_reg(p, REG_ADR, adr);
}

int cmd_write(struct context *p, char *data, int len)
{
    struct packet *b = &p->packet;
    packet_reset(b, 'w');
    packet_putci(b, data, len);
    return cmd_send(p);
}

int cmd_read(struct context *p, char *data, int len)
{
    struct packet *b = &p->packet;
    int ret;

    packet_reset(b, 'r');
    packet_putc(b, len);

    ret = cmd_send(p);
    if(ret && b->len == len) {
        memcpy(data, b->data, b->len);
        return 1;
    }

    return 0;
}

int process_file(struct context *p)
{
#define COPY_LENGTH 28
    char bufferw[COPY_LENGTH];
    char bufferr[COPY_LENGTH];
    char *id;
    uint32_t adr0, adr;
    int i, n;

    id = cmd_id(p);
    if(id) {
        printf("Connected to '%s'...\n", id);
    } else {
        printf("Communication failure\n");
        return 0;
    }


    cmd_set_adr(p, adr0 = adr = p->adr);

    while(!feof(p->fp)) {

        n = fread(bufferw, 1, COPY_LENGTH, p->fp);
        if(n <= 0) break;
        if(!cmd_write(p, bufferw, n))
            return 0;

        /* verify it */
        if(p->verify) {
            if(!cmd_set_adr(p, adr)) {
                fprintf(stderr, "set_adr failed\n");
                return 0;
            }
            if(!cmd_read(p, bufferr, n)) {
                fprintf(stderr, "read failed\n");
                return 0;
            }
            if(!memcpy(bufferw, bufferr + 1, n)) {
                fprintf(stderr, "readback failed\n");
                return 0;
            }
        }

        printf(" %08x   \r", adr);
        fflush(stdout);
        adr += n;
    }

    printf("\n");

    if(!p->dontrun) {
        printf("Starting at 0x%08x (options=%02x) ...\n",
               adr0, 0xFF & p->optstart);

        cmd_set_reg(p, REG_PC, adr0);
        printf("%s\n", cmd_start(p) ? "DONE" : "FAILED");
    }

    return 1;
}

int process_interactive(struct context *p)
{
    int n;

    for(;;) {
        fprintf(stdout, "> ");
        fflush(stdout);

        comm_file_recv(p);

        if(p->packet.cmd == 'q' && p->packet.len == 0)
            return 1;

        n = p->packet.len;
        util_recode(p->packet.data, &n);
        p->packet.len = n;


        if(p->packet.len > -1) {
            cmd_send(p);

        }
    }

    return 1;
}
