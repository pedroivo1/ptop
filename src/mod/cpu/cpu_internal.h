#ifndef CPU_INTERNAL_H
#define CPU_INTERNAL_H

#include "mod/cpu/cpu.h"
#include <stdint.h>

// --- BACKEND ---
void get_topology(CpuMon cpumon[static 1]);
void get_freq_mhz(CpuMon cpumon[static 1]);
void get_temp_c(CpuMon cpumon[static 1]);
void parse_stats(CpuMon cpumon[static 1]);
void get_load_avg(CpuMon cpumon[static 1]);
uint64_t get_temp_id();
void open_cpu_fds(CpuMon cpumon[static 1]);
void malloc_cpu(CpuMon cpumon[static 1]);

// --- FRONTEND ---
// UI
void draw_temp_ui(char* p[static 1], size_t x, size_t y);
void draw_freq_ui(char* p[static 1], size_t x, size_t y);
void draw_label_ui(char* p[static 1], unsigned id);
void draw_usage_ui(char* p[static 1], size_t width);
void draw_uptime_ui(char* p[static 1], size_t x, size_t y);
void draw_load_avg_ui(char* p[static 1], size_t x, size_t y);

// DATA
void draw_temp_data(char* p[static 1], size_t x, size_t y, int temp);
void draw_freq_data(char* p[static 1], size_t x, size_t y, unsigned mhz);
void draw_usage_data(char* p[static 1], unsigned usage);
void draw_uptime_data(char* p[static 1], size_t x, size_t y, unsigned uptime);
void draw_load_avg_data(char* p[static 1],
                        size_t x,
                        size_t y,
                        uint32_t avg[static 3]);

#endif
