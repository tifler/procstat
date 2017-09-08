#ifndef __PROCSTAT_H__
#define __PROCSTAT_H__

#define NR_CPU                          (16)
#define FLAG_VERBOSE                    (0x0001)
#define FLAG_DETAIL                     (0x0002)

/*****************************************************************************/

struct cpustat {
    unsigned long int user;
    unsigned long int nice;
    unsigned long int system;
    unsigned long int idle;
    unsigned long int iowait;
    unsigned long int irq;
    unsigned long int softirq;
    unsigned long int steal;
    unsigned long int guest;
    unsigned long int guest_nice;
};

struct procstat {
    struct cpustat cpu[NR_CPU + 1];
    int nr_cpu;
};

/*****************************************************************************/

int read_proc_stat(struct procstat *stbuf);
void show_proc_stat(struct procstat *, struct procstat *, unsigned long);

#endif  /*__PROCSTAT_H__*/
