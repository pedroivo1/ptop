#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <signal.h>
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

extern sig_atomic_t volatile g_signal_quit;

typedef struct {
    CpuMon cpu;
    MemMon mem;

    bool running;
    bool show_cpu;
    bool show_mem;
    bool needs_resize;
    bool force_redraw;

    uint16_t delay;
} AppContext;

void app_init(AppContext ctx[static 1]);
void app_destroy(AppContext ctx[static 1]);
void app_run(AppContext ctx[static 1]);

#endif
