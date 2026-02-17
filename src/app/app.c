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
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

void app_init(AppContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->delay = 500;
    ctx->running = 1;
    ctx->show_cpu = 1;
    ctx->show_mem = 1;
    ctx->needs_resize = 1;

    tui_setup(theme.bg, theme.fg);
    theme_init();

    init_cpu(&ctx->cpu);
    init_mem(&ctx->mem);

    app_update_layout(ctx);
}

void app_destroy(AppContext *ctx)
{
    deinit_cpu(&ctx->cpu);
    deinit_mem(&ctx->mem);

    tui_restore();
}

void app_run(AppContext *ctx)
{
    size_t buf_len = WRITE_BUF_LEN;
    char *buf = malloc(buf_len);
    if (!buf)
    {
        perror("malloc(buf_len) failed\nApp: app_run()");
        return;
    }

    uint64_t last_update_time = 0;
    uint64_t now;

    app_update_layout(ctx);

    while (ctx->running)
    {
        if (g_signal_quit) ctx->running = 0;

        now = current_time_ms();

        int is_tick = (last_update_time == 0 || now - last_update_time >= ctx->delay);

        if (is_tick)
        {
            last_update_time = now;
            app_update_state(ctx);
        }

        if (is_tick || ctx->needs_resize || ctx->force_redraw)
        {
            int bytes_written = app_draw(ctx, buf);

            if (bytes_written > 0)
                if (write(STDOUT_FILENO, buf, bytes_written) == -1) break;
        }

        int time_spent = (int)(current_time_ms() - last_update_time);
        int time_to_wait = ctx->delay - time_spent;
        if (time_to_wait < 0) time_to_wait = 0;

        app_handle_input(ctx, time_to_wait);
    }

    free(buf);
}
