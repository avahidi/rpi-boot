
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <getopt.h>

#include "defs.h"

#define UART_RATE B115200


void show_help(char *me, int fail)
{
    printf("Usage: %s [flags] <-o serialdevice> [-i input file] [-a address]\n"
           "Flags are:\n"
           "\t -n    don't run code after upload\n"
           "\t -W    start execution with watchdog enabled\n"
           "\t -H    start execution in hyp mode\n"
           "\t -S    start execution in secure mode\n"
           "\t -v    verify writes\n"
           "\t -d    debug communication\n"
           "\t -h    help: shows this message\n"
           , me);
    exit(fail ? 20 : 3);
}

void parse_options(struct context *c, int argc, char **argv)
{
    int ch, err = 0;
    while(!err && (ch = getopt (argc, argv, "HWShvndi:o:a:")) != -1) {
        switch(ch) {
        case 'n':
            c->dontrun = 1;
            break;
        case 'v':
            c->verify = 1;
            break;
        case 'W':
            c->optstart |= OPTSTART_WDOG;
            break;
        case 'H':
            c->optstart |= OPTSTART_HYP;
            break;
        case 'S':
            c->optstart |= OPTSTART_SEC;
            break;

        case 'd':
            c->debug = 1;
            break;
        case 'i':
            if(c->inname) err = 1;
            c->inname = optarg;
            break;
        case 'o':
            if(c->outname) err = 1;
            c->outname = optarg;
            break;
        case 'a':
            if(sscanf(optarg, "%x", &c->adr) != 1) {
                if(sscanf(optarg, "%d", &c->adr) != 1) {
                    fprintf(stderr, "Bad address: %s\n", optarg);
                    show_help(argv[0], 1);
                }
            }
            break;
        case 'h':
            show_help(argv[0], 0);
            break;
        default:
            err = 1;
            break;
        }
    }
    if(err || !c->outname)
        show_help(argv[0], 1);
}

int main(int argc , char **argv)
{
    int ret = 20;
    struct termios options;
    struct context p;

    bzero(&p, sizeof(p));
    parse_options(&p, argc, argv);

    if(!p.inname) {
        p.debug = 1;
        p.fp = stdin;
    } else {
        p.fp = fopen(p.inname, "rb");
    }

    if(p.fp) {
        if( (p.fd = open(p.outname, O_RDWR | O_NOCTTY | O_NDELAY )) != -1) {

            /* block on no data */
            fcntl(p.fd, F_SETFL, fcntl(p.fd, F_GETFL) & ~O_NONBLOCK);

            /* configure device */
            tcgetattr(p.fd, &options);
            cfsetispeed(&options, UART_RATE);
            cfsetospeed(&options, UART_RATE);
            options.c_cflag |= (CLOCAL | CREAD);
            if(tcsetattr(p.fd, TCSANOW, &options) != -1) {

                if(comm_dev_sync(&p)) {
                    ret = ((p.inname) ? process_file(&p) : process_interactive(&p))
                          ? 0 : 20;
                } else fprintf(stderr, "Could not sync to target\n");
            } else perror("Could not configure device");

            close(p.fd);
        } else perror("Could not open device");

        if(p.fp != stdin)
            fclose(p.fp);
    } else perror("Could not open input file");


    return ret;
}
