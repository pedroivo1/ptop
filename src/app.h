#ifndef APP_H
#define APP_H

#include <signal.h>
#include "modules/cpu.h"
#include "modules/mem.h"

extern volatile sig_atomic_t g_signal_quit;

typedef struct
{
    int x, y, w, h;
} Rect;

typedef struct
{
    CpuMon cpu;
    MemMon mem;

    int show_cpu;
    int show_mem;

    Rect r_cpu;
    Rect r_mem;

    int running;
    int needs_resize;
    int force_redraw;
} AppContext;

void app_init(AppContext *ctx);
void app_destroy(AppContext *ctx);
void update_layout(AppContext *ctx);
void handle_input(AppContext *ctx, int timeout_ms);

#endif
