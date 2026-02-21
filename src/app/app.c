#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "app/app.h"
#include "app/app_internal.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "util/util.h"
#include "cfg/buf.h"
#include "theme/theme.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

void app_init(AppContext ctx[static 1]) {
    memset(ctx, 0, sizeof(*ctx));

    ctx->delay = 300;
    ctx->running = true;
    ctx->show_cpu = true;
    ctx->show_mem = false;
    ctx->needs_resize = true;

    tui_setup(theme.bg, theme.fg);
    set_theme(THEME_DARK);

    init_cpu(&ctx->cpu);
    init_mem(&ctx->mem);

    app_update_layout(ctx);
}

void app_destroy(AppContext ctx[static 1]) {
    deinit_cpu(&ctx->cpu);
    deinit_mem(&ctx->mem);

    tui_restore();
}

void app_run(AppContext ctx[static 1]) {
    app_update_layout(ctx);

    char buf[WRITE_BUF_LEN];
    uint64_t last_update_time = 0;
    while (ctx->running) {
        if (g_signal_quit) {
            ctx->running = false;
        }

        uint64_t now = current_time_ms();

        int is_tick = (last_update_time == 0 || now - last_update_time >= ctx->delay);

        if (is_tick) {
            last_update_time = now;
            app_update_state(ctx);
        }

        if (is_tick || ctx->needs_resize || ctx->force_redraw) {
            ptrdiff_t bytes_written = app_draw(ctx, buf);

            if (bytes_written > 0) {
                if (write(STDOUT_FILENO, buf, bytes_written) == -1) {
                    break;
                }
            }
        }

        uint64_t time_spent = current_time_ms() - last_update_time;
        int time_to_wait = ctx->delay - time_spent;
        if (time_to_wait < 0) {
            time_to_wait = 0;
        }

        app_handle_input(ctx, time_to_wait);
    }
}
