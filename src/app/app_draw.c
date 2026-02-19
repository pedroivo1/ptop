#include <stddef.h>
#include "app.h"
#include "app_internal.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "util/util.h"
#include "theme/theme.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

void draw_delay_ctl(AppContext ctx[static 1], char* p[static 1]) {

    int x = term_w - 12; 
    int y = 1;

    tui_at(p, x, y);

    append_str(p, theme.cpu_bd);
    APPEND_LIT(p, BOX_TR);
    append_str(p, theme.fg);
    APPEND_LIT(p, "- ");

    append_num(p, ctx->delay);
    APPEND_LIT(p, "ms");

    APPEND_LIT(p, " +");
    append_str(p, theme.cpu_bd);
    APPEND_LIT(p, BOX_TL);
    append_str(p, theme.fg);
}

ptrdiff_t app_draw(AppContext ctx[static 1], char buf[static 1]) {
    char* p = buf;
    bool physical_resize = false;

    tui_begin_frame(&physical_resize);

    if (physical_resize) {
        ctx->needs_resize = true;
        app_update_layout(ctx);
    }

    bool draw_static = ctx->force_redraw || ctx->needs_resize;

    if (draw_static) {
        append_str(&p, theme.bg);
        APPEND_LIT(&p, "\033[2J");
    }

    if (ctx->show_cpu) {
        if (draw_static) {
            draw_cpu_ui(&ctx->cpu, &p);
        }

        draw_cpu_data(&ctx->cpu, &p);
    }

    if (ctx->show_mem) {
        if (draw_static) {
            draw_mem_ui(&ctx->mem, &p);
        }

        draw_mem_data(&ctx->mem, &p);
    }

    if (draw_static) {
        draw_delay_ctl(ctx, &p);
    }

    ctx->force_redraw = false;
    ctx->needs_resize = false;

    return p - buf;
}
