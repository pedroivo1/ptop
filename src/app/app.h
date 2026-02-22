#ifndef APP_H
#define APP_H

#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"
#include <signal.h>

extern sig_atomic_t volatile g_signal_quit;

void app_init(CpuMon cpu[static 1], MemMon mem[static 1]);
void app_run(CpuMon cpu[static 1],
             MemMon mem[static 1],
             unsigned delay[static 1]);
void app_destroy(CpuMon cpu[static 1], MemMon mem[static 1]);

#endif
