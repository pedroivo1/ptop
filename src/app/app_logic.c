#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include "app.h"
#include "app_internal.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

void app_update_layout(AppContext ctx[static 1]) {
    int w = term_w;
    int h = term_h;
    int margin = 1;

    if (ctx->show_cpu && ctx->show_mem) {
        int split_h = h - 7;
        ctx->cpu.rect = (Rect){margin, margin, w, split_h};
        ctx->mem.rect = (Rect){margin, margin + split_h, w, h - split_h};
    } else if (ctx->show_cpu && !ctx->show_mem) {
        ctx->cpu.rect = (Rect){margin, margin, w, h};
    } else if (!ctx->show_cpu && ctx->show_mem) {
        ctx->mem.rect = (Rect){margin, margin, w, h};
    }

    if (ctx->show_cpu) {
        recalc_cpu(&ctx->cpu);
    }

    ctx->force_redraw = 1;
    ctx->needs_resize = 0;
}

static void handle_mouse(AppContext ctx[static 1]) {
    char seq[32];
    int len = 0;
    struct pollfd pfd_drain = { STDIN_FILENO, POLLIN, 0 };
    
    while(poll(&pfd_drain, 1, 0) > 0 && len < 31) {
        if (read(STDIN_FILENO, &seq[len], 1) <= 0) {
            break;
        }
        len++;
    }
    seq[len] = '\0';

    if (len > 3 && seq[0] == '[' && seq[1] == '<') {
        int btn, mx, my;
        char type;
        
        if (sscanf(seq + 2, "%d;%d;%d%c", &btn, &mx, &my, &type) == 4) {
            
            if (type == 'M' && btn == 0) {
                
                int base_x = term_w - 12;
                int base_y = 1;

                if (my == base_y) {
                    
                    if (mx >= base_x && mx <= base_x + 2) {
                        if (ctx->delay > 100) {
                            ctx->delay -= 100;
                        } else {
                            ctx->delay = 100;
                        }
                        ctx->force_redraw = 1;
                    }
                    
                    int num_len = (ctx->delay >= 1000) ? 4 : 3;
                    int plus_offset = 3 + num_len + 2 + 1;

                    if (mx >= base_x + plus_offset && mx <= base_x + plus_offset + 1) {
                        if (ctx->delay < 9900) {
                            ctx->delay += 100;
                        }
                        ctx->force_redraw = 1;
                    }
                }
            }
        }
    }
}

static void handle_keyboard(AppContext ctx[static 1], char key) {
    switch (key) {
        case 0x03: // CTRL+C
        case 'q':
        case 'Q':
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
        case 't':
        case 'T':
            theme_toggle();
            ctx->force_redraw = 1;
            break;
        case 'r':
        case 'R':
            ctx->force_redraw = 1;
            break;
    }
}

void app_handle_input(AppContext ctx[static 1], int timeout_ms) {
    if (g_signal_quit) {
        ctx->running = 0;
        return;
    }

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, timeout_ms);

    if (ret > 0 && (fds[0].revents & POLLIN)) {
        char key;
        if (read(STDIN_FILENO, &key, 1) > 0) {
            if (key == 0x1B) {
                handle_mouse(ctx);
            } else {
                handle_keyboard(ctx, key);
            }
        }

    } else if (ret < 0 && g_signal_quit) {
        ctx->running = 0;
    }
}

void app_update_state(AppContext ctx[static 1])
{
    if (ctx->show_cpu){
        update_cpu_data(&ctx->cpu);
    }
    if (ctx->show_mem){
        update_mem_data(&ctx->mem);
    }
}
