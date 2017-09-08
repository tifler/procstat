#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "procstat.h"

/*****************************************************************************/

#define ALOGE       printf
#define ASSERT      assert

/*****************************************************************************/

enum {
    CPU_STAT_USER,
    CPU_STAT_NICE,
    CPU_STAT_SYSTEM,
    CPU_STAT_IDLE,
    CPU_STAT_IOWAIT,
    CPU_STAT_IRQ,
    CPU_STAT_SOFTIRQ,
    CPU_STAT_STEAL,
    CPU_STAT_GUEST,
    CPU_STAT_GUEST_NICE,
    CPU_STAT_NR,
};

/*****************************************************************************/

struct cpustat_arr {
    unsigned long int stat[CPU_STAT_NR];
};

/*****************************************************************************/

static int proc_read(const char *path, char *s, int num_bytes)
{
    char buf[80];
    int count;
    int ret = 0;
    int fd = open(path, O_RDONLY);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error opening %s: %s\n", path, buf);
        return -1;
    }

    if ((count = read(fd, s, num_bytes - 1)) < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGE("Error writing to %s: %s\n", path, buf);
        ret = -1;
    } else {
        s[count] = '\0';
    }

    close(fd); 

    return ret;
}

static int parse_proc_stat(struct procstat *stbuf, char *strbuf)
{
    int cpuid;
    char *s, *e;
    struct cpustat *cpu;

    for (s = strbuf, cpuid = 0; ; s = e) {
        if (!s)
            break;

        e = strchr(s, '\n');
        if (e) {
            *e = 0;
            e++;
        }
        if (strncmp(s, "cpu", 3))
            continue;

        s = strchr(s, ' ');
        ASSERT(s);
        //printf("String => [%s]\n", s);

        cpu = &stbuf->cpu[cpuid];
        sscanf(s, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", 
                &cpu->user,
                &cpu->nice,
                &cpu->system,
                &cpu->idle,
                &cpu->iowait,
                &cpu->irq,
                &cpu->softirq,
                &cpu->steal,
                &cpu->guest,
                &cpu->guest_nice);
#if 0
        printf("CPU%d %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", 
                cpuid,
                cpu->user,
                cpu->nice,
                cpu->system,
                cpu->idle,
                cpu->iowait,
                cpu->irq,
                cpu->softirq,
                cpu->steal,
                cpu->guest,
                cpu->guest_nice);
#endif  /*0*/
        cpuid++;
    }

    stbuf->nr_cpu = cpuid - 1;
    //printf("nr_cpu = %d\n", stbuf->nr_cpu);

    return 0;
}

static unsigned long int get_proc_sum(struct cpustat *cpu)
{
    int i;
    unsigned long int sum = 0;
    struct cpustat_arr *arr = (struct cpustat_arr *)cpu;

    for (i = 0; i < CPU_STAT_NR; i++)
        sum += arr->stat[i];

    return sum;
}

/*****************************************************************************/

int read_proc_stat(struct procstat *stbuf)
{
    int ret;
    char procbuf[4096];

    ret = proc_read("/proc/stat", procbuf, sizeof(procbuf));
    ASSERT(ret == 0);

    //printf("BUF ---\n%s\n-----\n", procbuf);

    ret = parse_proc_stat(stbuf, procbuf);
    ASSERT(ret == 0);

    return ret;
}

void show_proc_stat(struct procstat *prev, struct procstat *curr, unsigned long flags)
{
    int i;
    unsigned long int sum;
    unsigned long int idle;

    sum = get_proc_sum(&curr->cpu[0]) - get_proc_sum(&prev->cpu[0]);
    idle = curr->cpu[0].idle - prev->cpu[0].idle;
    printf("%3.1f", (float)(sum - idle) / (float)sum * 100.0f);

    if (flags & FLAG_VERBOSE)
        printf(" (%lu/%lu)", sum - idle, sum);

    if (flags & FLAG_DETAIL)
        printf(" ----------------------------------------------------------------------");
    else
        printf(":");

    for (i = 0; i < prev->nr_cpu; i++) {
        if (flags & FLAG_DETAIL)
            printf("\n[%2d] ", i);
        else
            printf(" ");
        sum = get_proc_sum(&curr->cpu[i + 1]) - get_proc_sum(&prev->cpu[i + 1]);
        idle = curr->cpu[i + 1].idle - prev->cpu[i + 1].idle;
        printf("%5.1f", (float)(sum - idle) / (float)sum * 100.0f);
        if (flags & FLAG_VERBOSE) {
            printf(" (%3lu/%3lu", sum - idle, sum);
            if (flags & FLAG_DETAIL) {
                printf("  idle:%3ld, usr:%3ld, nic:%3ld, sys:%3ld, iow:%3ld, irq:%3ld, sirq:%3ld",
                        curr->cpu[i + 1].idle - prev->cpu[i + 1].idle,
                        curr->cpu[i + 1].user - prev->cpu[i + 1].user,
                        curr->cpu[i + 1].nice - prev->cpu[i + 1].nice,
                        curr->cpu[i + 1].system - prev->cpu[i + 1].system,
                        curr->cpu[i + 1].iowait - prev->cpu[i + 1].iowait,
                        curr->cpu[i + 1].irq - prev->cpu[i + 1].irq,
                        curr->cpu[i + 1].softirq - prev->cpu[i + 1].softirq);
            }
            printf(")");
        }
    }
    printf("\n");
}
