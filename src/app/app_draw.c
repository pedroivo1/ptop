#include "app.h"
#include "app_internal.h"
#include "ui/ui.h"
#include "common/utils.h"

int app_draw(AppContext *ctx, char *buf)
{
    char *p = buf;

    int physical_resize = 0;
    p = tui_begin_frame(p, &physical_resize);

    if (physical_resize) {
        ctx->needs_resize = 1;
        app_update_layout(ctx);
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
