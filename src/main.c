#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdalign.h>
#include <time.h>

#include "app.h"
#include "utils.h"
#include "tui.h"
#include "cfg.h"

volatile sig_atomic_t g_signal_quit = 0;

void handle_sigint(int sig)
{
    (void)sig;
    g_signal_quit = 1;
}

int main()
{
    signal(SIGINT, handle_sigint);

    AppContext ctx;
    app_init(&ctx);

    tui_setup(BG_BLACK, WHITE);
    static alignas(64) char buf[OUT_BUFF_LEN];

    uint64_t last_update_time = 0;
    uint64_t now;

    while (ctx.running)
    {
        now = current_time_ms();

        int update_data_tick = 0;
        if (now - last_update_time >= DELAY_MS)
        {
            update_data_tick = 1;
            last_update_time = now;
        }

        char *p = buf;

        p = tui_begin_frame(p, &ctx.needs_resize);
        if (ctx.needs_resize) update_layout(&ctx);

        if (ctx.force_redraw && !ctx.needs_resize) p = append_str(p, "\033[2J");

        if (update_data_tick)
        {
            if (ctx.show_cpu) update_cpu_data(&ctx.cpu);
            if (ctx.show_mem) update_mem_data(&ctx.mem);
        }

        if (update_data_tick || ctx.needs_resize || ctx.force_redraw)
        {
            int draw_static = ctx.force_redraw || ctx.needs_resize;

            if (ctx.show_cpu)
            {
                if (draw_static)
                    p = draw_cpu_ui(p, ctx.r_cpu.x, ctx.r_cpu.y, ctx.r_cpu.w, ctx.r_cpu.h);

                p = draw_cpu_data(&ctx.cpu, p, ctx.r_cpu.x, ctx.r_cpu.y, ctx.r_cpu.w, ctx.r_cpu.h);
            }

            if (ctx.show_mem)
            {
                if (draw_static)
                    p = draw_mem_ui(p, ctx.r_mem.x, ctx.r_mem.y, ctx.r_mem.w, ctx.r_mem.h);

                p = draw_mem_data(&ctx.mem, p, ctx.r_mem.x, ctx.r_mem.y, ctx.r_mem.w, ctx.r_mem.h);
            }

            ctx.force_redraw = 0;

            if (write(STDOUT_FILENO, buf, p - buf) == -1) break;
        }

        int time_spent = (int)(current_time_ms() - last_update_time);
        int time_to_wait = DELAY_MS - time_spent;
        if (time_to_wait < 0) time_to_wait = 0;

        handle_input(&ctx, time_to_wait);
    }

    app_destroy(&ctx);
    tui_restore();
    return 0;
}
