#include "app.h"
#include "app_internal.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "util/util.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"

void draw_delay_ctl(AppContext *ctx, char **p);
int app_draw(AppContext *ctx, char *buf)
{
    char *p = buf;
    int physical_resize = 0;

    tui_begin_frame(&p, &physical_resize);

    if (physical_resize) {
        ctx->needs_resize = 1;
        app_update_layout(ctx);
    }

    if (ctx->force_redraw && !ctx->needs_resize) {
        APPEND_LIT(&p, "\033[2J");
    }

    int draw_static = ctx->force_redraw || ctx->needs_resize;

    if (ctx->show_cpu)
    {
        if (draw_static)
            draw_cpu_ui(&ctx->cpu, &p);

        draw_cpu_data(&ctx->cpu, &p);
    }

    if (ctx->show_mem)
    {
        if (draw_static)
            draw_mem_ui(&ctx->mem, &p);

        draw_mem_data(&ctx->mem, &p);
    }

    if (draw_static)
        draw_delay_ctl(ctx, &p);

    ctx->force_redraw = 0;
    ctx->needs_resize = 0;

    return (int)(p - buf);
}

void draw_delay_ctl(AppContext *ctx, char **p)
{

    int x = term_w - 12; 
    int y = 1;

    tui_at(p, x, y);
    
    APPEND_LIT(p, TC_CPU_BD BOX_TR TX_FONT "- ");

    append_num(p, ctx->delay);
    APPEND_LIT(p, "ms");

    APPEND_LIT(p, " +" TC_CPU_BD BOX_TL TX_FONT);

}
