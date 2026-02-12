#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include "../cfg.h"
#include "rect.h"

#define CPU_BORDER_C "\033[38;5;65m"

typedef struct
{
    uint64_t *prev_total;
    uint64_t *prev_idle;

    int *fd_freq;

    uint8_t *usage;
    uint8_t *graph_hist;

    uint32_t uptime;
    uint32_t load_avg[3];

    int fd_stat;
    int fd_temp;

    uint16_t freq;
    uint16_t core_count;

    Rect rect;
    Rect r_table;

    int8_t temp;
    uint8_t graph_head;

} CpuMon;

void init_cpu(CpuMon *cpumon);
void deinit_cpu(CpuMon *cpumon);
void update_cpu_data(CpuMon *cpumon);

void cpu_recalc(CpuMon *cpumon);

char *draw_cpu_ui(CpuMon *cpumon, char *p);
char *draw_cpu_data(CpuMon* cpumon, char *p);

#endif
