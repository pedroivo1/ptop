#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <signal.h>
#include "modules/cpu.h"
#include "modules/mem.h"

extern volatile sig_atomic_t g_signal_quit;

typedef struct
{
    int x, y, w, h;
} Rect;

typedef struct {
    int running;
    int show_cpu;
    int show_mem;
    int needs_resize;
    int force_redraw;

    Rect r_cpu;
    Rect r_mem;

    CpuMon cpu;
    MemMon mem;

} AppContext;

void app_init(AppContext *ctx);
void app_destroy(AppContext *ctx);
void update_layout(AppContext *ctx);
void handle_input(AppContext *ctx, int timeout_ms);

void app_update_state(AppContext *ctx);
int app_render_frame(AppContext *ctx, char *buffer);

#endif
