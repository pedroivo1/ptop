#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#include "app.h"
#include "tui.h"
#include "cfg.h"
#include "utils.h"

void app_init(AppContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    ctx->running = 1;
    ctx->show_cpu = 1;
    ctx->show_mem = 1;
    ctx->needs_resize = 1;

    init_cpu(&ctx->cpu);
    init_mem(&ctx->mem);

    update_layout(ctx);
}

void app_destroy(AppContext *ctx)
{
    deinit_cpu(&ctx->cpu);
    deinit_mem(&ctx->mem);
}

void update_layout(AppContext *ctx)
{
    int w = term_w;
    int h = term_h;
    int margin = 1;

    ctx->cpu.rect = (Rect){0, 0, 0, 0};
    ctx->mem.rect = (Rect){0, 0, 0, 0};

    if (ctx->show_cpu && ctx->show_mem)
    {
        int split_h = h - 7;
        if (split_h < 10) split_h = h / 2;

        ctx->cpu.rect = (Rect){margin, margin, w, split_h};
        ctx->mem.rect = (Rect){margin, margin + split_h, w, h - split_h};
    }
    else if (ctx->show_cpu && !ctx->show_mem)
    {
        ctx->cpu.rect = (Rect){margin, margin, w, h};
    }
    else if (!ctx->show_cpu && ctx->show_mem)
    {
        ctx->mem.rect = (Rect){margin, margin, w, h};
    }

    if (ctx->show_cpu)
    {
        cpu_recalc(&ctx->cpu);
    }

    ctx->force_redraw = 1;
    ctx->needs_resize = 0;
}

void app_handle_input(AppContext *ctx, int timeout_ms)
{
    if (g_signal_quit)
    {
        ctx->running = 0;
        return;
    }

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, timeout_ms);

    if (ret > 0 && (fds[0].revents & POLLIN))
    {
        char key;
        if (read(STDIN_FILENO, &key, 1) > 0)
        {
            if (key == 0x1B)
            {
                char trash[128];
                struct pollfd pfd_drain = { STDIN_FILENO, POLLIN, 0 };

                while(poll(&pfd_drain, 1, 0) > 0) {
                    if (read(STDIN_FILENO, trash, sizeof(trash)) <= 0) break;
                }
                return;
            }

            switch (key)
            {
                case 'q':
                case 0x03: // CTRL+C
                    ctx->running = 0;
                    break;

                case '1':
                    ctx->show_cpu = !ctx->show_cpu;
                    update_layout(ctx);
                    break;

                case '2':
                    ctx->show_mem = !ctx->show_mem;
                    update_layout(ctx);
                    break;

                case 'r':
                    ctx->force_redraw = 1;
                    break;
            }
        }
    }
    else if (ret < 0)
    {
        if (g_signal_quit) ctx->running = 0;
    }
}

void app_update_state(AppContext *ctx)
{
    if (ctx->show_cpu) update_cpu_data(&ctx->cpu);
    if (ctx->show_mem) update_mem_data(&ctx->mem);
}

int app_render_frame(AppContext *ctx, char *buf)
{
    char *p = buf;

    int physical_resize = 0;
    p = tui_begin_frame(p, &physical_resize);

    if (physical_resize) {
        ctx->needs_resize = 1;
        update_layout(ctx);
    }

    if (ctx->force_redraw && !ctx->needs_resize) {
        p = append_str(p, "\033[2J");
    }

    int draw_static = ctx->force_redraw || ctx->needs_resize;

    if (ctx->show_cpu)
    {
        if (draw_static)
            p = draw_cpu_ui(&ctx->cpu, p);

        p = draw_cpu_data(&ctx->cpu, p);
    }

    if (ctx->show_mem)
    {
        if (draw_static)
            p = draw_mem_ui(&ctx->mem, p);

        p = draw_mem_data(&ctx->mem, p);
    }

    ctx->force_redraw = 0;
    ctx->needs_resize = 0;

    return (int)(p - buf);
}
