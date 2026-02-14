#include <fcntl.h>

#include "common/cfg.h"
#include "ui/ui.h"
#include "modules/mem/mem_internal.h"

void init_mem(MemMon *memmon)
{
    memset(memmon, 0, sizeof(*memmon));
    memmon->fd_mem = open(MEM_PATH, O_RDONLY);
}

void deinit_mem(MemMon *memmon)
{
    close(memmon->fd_mem);
}

void update_mem_data(MemMon *memmon)
{
    parse_mem(memmon);
}

char *draw_mem_ui(MemMon *memmon, char *p)
{
    int x = memmon->rect.x;
    int y = memmon->rect.y;
    int w = memmon->rect.w;
    int h = memmon->rect.h;

    // --- MAIN BOX ---
    p = tui_draw_box(p, x, y, w, h, TC_MEM_BD);
    p = append_str(p, TX_FONT);

    return p;
}

char *draw_mem_data(MemMon *memmon, char *p)
{
    int x = memmon->rect.x;
    int y = memmon->rect.y;
    int w = memmon->rect.w;
    int h = memmon->rect.h;

    (void)h;
    int start_x = x + 2;
    int current_y = y + 1;
    int inner_w = w - 4;

    p = tui_at(p, start_x, current_y++);
    p = append_str(p, "\033[1mRAM\033[22m Total: ");

    p = append_fixed2(p, memmon->total, 20, 100);
    p = append_str(p, " GiB");

    if (memmon->total == 0) return p;

    int p_used   = (memmon->used   * 100) / memmon->total;
    int p_avail  = (memmon->available * 100) / memmon->total;
    int p_cached = (memmon->cached * 100) / memmon->total;
    int p_free   = (memmon->free   * 100) / memmon->total;

    p = draw_meter(p, start_x, current_y++, inner_w, "Used ", TC_MEM_USED,  p_used,   memmon->used);
    p = draw_meter(p, start_x, current_y++, inner_w, "Avail", TC_MEM_AVAIL, p_avail,  memmon->available);
    p = draw_meter(p, start_x, current_y++, inner_w, "Cache", TC_MEM_CACHE, p_cached, memmon->cached);
    p = draw_meter(p, start_x, current_y++, inner_w, "Free ", TC_MEM_FREE,  p_free,   memmon->free);

    return p;
}
