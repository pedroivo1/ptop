#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include "../cfg.h"

#define CPU_BORDER_C "\033[38;5;65m"

typedef struct
{
    uint64_t prev_total[CORES_N];
    uint64_t prev_idle[CORES_N];

    uint32_t uptime;
    uint32_t load_avg[3];

    int fd_stat;
    int fd_temp;
    int fd_freq[PHY_CORES_N];

    uint16_t freq;

    uint8_t usage[CORES_N];
    int8_t temp;
    uint8_t graph_head;
    uint8_t graph_hist[CORES_N][GRAPH_WIDTH];

} CpuMonitor;

void init_cpumon(CpuMonitor *cpumon);
void cleanup_cpumon(CpuMonitor *cpumon);
int get_coretemp_id();
void update_cpu_metrics(CpuMonitor *cpumon);
char *render_interface(CpuMonitor* cpumon, char *p, int x, int y, int w, int h);

#endif
