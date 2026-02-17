#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <signal.h>

#include "modules/cpu/cpu.h"
#include "modules/mem/mem.h"
#include "util/rect.h"

extern volatile sig_atomic_t g_signal_quit;

typedef struct {
    CpuMon cpu;
    MemMon mem;

    int running;
    int show_cpu;
    int show_mem;
    int needs_resize;
    int force_redraw;

    uint16_t delay;
} AppContext;

void app_init(AppContext *ctx);
void app_destroy(AppContext *ctx);
void app_run(AppContext *ctx);

#endif
