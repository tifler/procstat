#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "procstat.h"

static void usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            " -i <sec>  Interval in seconds\n"
            " -v        Display verbosely\n"
            " -h        Help\n"
            , program);
}

int main(int argc, char **argv)
{
    int opt;
    int interval = 1;
    unsigned long flags = 0;
    struct procstat prev, curr;

    while ((opt = getopt(argc, argv, "i:vh")) != -1) {
        switch (opt) {
            case 'i':
                interval = atoi(optarg);
                break;
            case 'v':
                if (flags & FLAG_VERBOSE)
                    flags |= FLAG_DETAIL;
                else
                    flags |= FLAG_VERBOSE;
                break;
            case 'h':
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    read_proc_stat(&prev);

    for ( ; ; ) {
        sleep (interval);
        read_proc_stat(&curr);
        show_proc_stat(&prev, &curr, flags);
        prev = curr;
    }

    return EXIT_SUCCESS;
}
