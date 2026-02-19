#ifndef CPU_INTERNAL_H
#define CPU_INTERNAL_H

#include <stdint.h>
#include "mod/cpu/cpu.h"

// --- BACKEND ---
void get_topology(CpuMon cpumon[static 1]);
void get_freq_mhz(CpuMon cpumon[static 1]);
void get_temp_c(CpuMon cpumon[static 1]);
void parse_stats(CpuMon cpumon[static 1]);
void get_load_avg(CpuMon cpumon[static 1]);
int get_temp_id();

// --- FRONTEND ---
// UI
void draw_temp_ui(char* p[static 1], int x, int y);
void draw_freq_ui(char* p[static 1], int x, int y);
void draw_label_ui(char* p[static 1], int id);
void draw_usage_ui(char* p[static 1], size_t width);
void draw_uptime_ui(char* p[static 1], int x, int y);
void draw_load_avg_ui(char* p[static 1], int x, int y);

// DATA
void draw_temp_data(char* p[static 1], int x, int y, int temp);
void draw_freq_data(char* p[static 1], int x, int y, int mhz);
void draw_usage_data(char* p[static 1], int usage);
void draw_uptime_data(char* p[static 1], int x, int y, int uptime);
void draw_load_avg_data(char* p[static 1], int x, int y, uint32_t avg[3]);

#endif
