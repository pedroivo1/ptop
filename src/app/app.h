#ifndef APP_H
#define APP_H

#include <signal.h>
#include "modules/cpu/cpu.h"
#include "modules/mem/mem.h"
#include "common/rect.h"

extern volatile sig_atomic_t g_signal_quit;

typedef struct {
    int running;
    int show_cpu;
    int show_mem;
    int needs_resize;
    int force_redraw;

    CpuMon cpu;
    MemMon mem;
} AppContext;

void app_init(AppContext *ctx);
void app_destroy(AppContext *ctx);
void app_run(AppContext *ctx);

#endif
