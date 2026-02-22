#ifndef CPU_H
#define CPU_H

#include "util/rect.h"
#include <stddef.h>
#include <stdint.h>

typedef struct
{
   uint64_t* prev_total;
   uint64_t* prev_idle;

   int* fd_freq;

   uint8_t* usage;
   uint8_t* graph_hist;

   char* stat_buffer;
   size_t stat_buffer_size;

   uint64_t main_prev_total;
   uint64_t main_prev_idle;

   uint32_t uptime;
   uint32_t load_avg[3];

   int fd_stat;
   int fd_temp;

   uint16_t freq;
   uint16_t phy_count;
   uint16_t thread_count;

   Rect rect;
   Rect r_table;
   Rect r_graph;

   uint16_t table_cols;
   uint16_t table_rows;
   uint16_t col_width;

   uint8_t threads_per_core;
   int8_t temp;
   uint8_t graph_head;
   uint8_t graph_width;

   bool is_compact;

   uint8_t main_graph_head;
   uint8_t main_usage[256];
} CpuMon;

void init_cpu(CpuMon cpumon[static 1]);
void deinit_cpu(CpuMon cpumon[static 1]);

void update_cpu(CpuMon cpumon[static 1]);

void calc_cpu_layout(CpuMon cpumon[static 1]);
void draw_cpu_ui(CpuMon cpumon[static 1], char* p[static 1]);
void draw_cpu_data(CpuMon cpumon[static 1], char* p[static 1]);

#endif
