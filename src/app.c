#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#include "app.h"
#include "tui.h"
#include "cfg.h"

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

    ctx->r_cpu = (Rect){0, 0, 0, 0};
    ctx->r_mem = (Rect){0, 0, 0, 0};

    if (ctx->show_cpu && ctx->show_mem)
    {
        int split_h = h - 7;
        if (split_h < 10) split_h = h / 2;

        ctx->r_cpu = (Rect){margin, margin, w, split_h};
        ctx->r_mem = (Rect){margin, margin + split_h, w, h - split_h};
    }
    else if (ctx->show_cpu && !ctx->show_mem)
    {
        ctx->r_cpu = (Rect){margin, margin, w, h};
    }
    else if (!ctx->show_cpu && ctx->show_mem)
    {
        ctx->r_mem = (Rect){margin, margin, w, h};
    }

    ctx->force_redraw = 1;
    ctx->needs_resize = 0;
}

void handle_input(AppContext *ctx, int timeout_ms)
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
            switch (key)
            {
                case 'q':
                case 0x03:
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
