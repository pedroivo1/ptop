#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include "../cfg.h"
#include "rect.h"

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

    Rect rect;
    Rect r_table;

    uint8_t usage[CORES_N];
    int8_t temp;
    uint8_t graph_head;
    uint8_t graph_hist[CORES_N][GRAPH_WIDTH];

} CpuMon;

void init_cpu(CpuMon *cpumon);
void deinit_cpu(CpuMon *cpumon);
void update_cpu_data(CpuMon *cpumon);

void cpu_recalc(CpuMon *cpumon);

char *draw_cpu_ui(CpuMon *cpumon, char *p);
char *draw_cpu_data(CpuMon* cpumon, char *p);

#endif
