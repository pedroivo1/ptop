#ifndef CPU_INTERNAL_H
#define CPU_INTERNAL_H

#include <stdint.h>

#include "modules/cpu/cpu.h"

// --- BACKEND ---
void get_topology(CpuMon *cpumon);
void get_freq_mhz(CpuMon *cpumon);
void get_temp_c(CpuMon *cpumon);
void parse_stats(CpuMon *cpumon);
void get_load_avg(CpuMon *cpumon);
int get_temp_id();

// --- FRONTEND ---
// UI 
void draw_temp_ui(char **p, int x, int y);
void draw_freq_ui(char **p, int x, int y);
void draw_label_ui(char **p, int id);
void draw_usage_ui(char **p, int width);
void draw_uptime_ui(char **p, int x, int y);
void draw_load_avg_ui(char **p, int x, int y);

// DATA
void draw_temp_data(char **p, int x, int y, int temp);
void draw_freq_data(char **p, int x, int y, int mhz);
void draw_usage_data(char **p, int usage);
void draw_uptime_data(char **p, int x, int y, int uptime);
void draw_load_avg_data(char **p, int x, int y, uint32_t avg[3]);
#endif
