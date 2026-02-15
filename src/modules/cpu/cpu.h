#ifndef CPU_H
#define CPU_H

#include <stddef.h>
#include <stddef.h>

#include "common/rect.h"

typedef struct
{
    uint64_t *prev_total;
    uint64_t *prev_idle;

    int *fd_freq;

    uint8_t *usage;
    uint8_t *graph_hist;

    char *stat_buffer;
    size_t stat_buffer_size;

    uint32_t uptime;
    uint32_t load_avg[3];

    int fd_stat;
    int fd_temp;

    uint16_t freq;
    uint16_t phy_count;
    uint16_t thread_count;

    Rect rect;
    Rect r_table;

    uint8_t threads_per_core;
    int8_t temp;
    uint8_t graph_head;

} CpuMon;

void init_cpu(CpuMon *cpumon);
void deinit_cpu(CpuMon *cpumon);

void update_cpu_data(CpuMon *cpumon);

void recalc_cpu(CpuMon *cpumon);
void draw_cpu_ui(CpuMon *cpumon, char **p);
void draw_cpu_data(CpuMon* cpumon, char **p);

#endif
