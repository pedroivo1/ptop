#include <poll.h>
#include <stdio.h>
#include <unistd.h>

#include "app.h"
#include "app_internal.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "modules/cpu/cpu.h"
#include "modules/mem/mem.h"

void app_update_layout(AppContext *ctx)
{
    int w = term_w;
    int h = term_h;
    int margin = 1;

    ctx->cpu.rect = (Rect){0, 0, 0, 0};
    ctx->mem.rect = (Rect){0, 0, 0, 0};

    if (ctx->show_cpu && ctx->show_mem)
    {
        int split_h = h - 7;

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
        recalc_cpu(&ctx->cpu);
    }

    ctx->force_redraw = 1;
    ctx->needs_resize = 0;
}

void app_handle_input(AppContext *ctx, int timeout_ms)
{
    if (g_signal_quit) {
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
            if (key == 0x1B) // ESC sequence handling
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
                case 'Q':
                case 0x03: // CTRL+C
                    ctx->running = 0;
                    break;
                case '1':
                    ctx->show_cpu = !ctx->show_cpu;
                    app_update_layout(ctx);
                    break;
                case '2':
                    ctx->show_mem = !ctx->show_mem;
                    app_update_layout(ctx);
                    break;
                case 'r':
                    ctx->force_redraw = 1;
                    break;
            }
        }
    }
    else if (ret < 0 && g_signal_quit)
    {
        ctx->running = 0;
    }
}

void app_update_state(AppContext *ctx)
{
    if (ctx->show_cpu) update_cpu_data(&ctx->cpu);
    if (ctx->show_mem) update_mem_data(&ctx->mem);
}
