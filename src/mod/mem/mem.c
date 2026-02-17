#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "util/util.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "cfg/path.h"
#include "mod/mem/mem_internal.h"

void init_mem(MemMon *memmon)
{
    memset(memmon, 0, sizeof(*memmon));
    memmon->fd_mem = open(MEMINFO, O_RDONLY);
}

void deinit_mem(MemMon *memmon)
{
    close(memmon->fd_mem);
}

void update_mem_data(MemMon *memmon)
{
    parse_mem(memmon);
}

void draw_mem_ui(MemMon *memmon, char **p)
{
    int x = memmon->rect.x;
    int y = memmon->rect.y;
    int w = memmon->rect.w;
    int h = memmon->rect.h;

    // --- MAIN BOX ---
    tui_draw_box(p, x, y, w, h, TC_MEM_BD);
    APPEND_LIT(p, TX_FONT);
}

void draw_mem_data(MemMon *memmon, char **p)
{
    int x = memmon->rect.x;
    int y = memmon->rect.y;
    int w = memmon->rect.w;

    int start_x = x + 2;
    int current_y = y + 1;
    int inner_w = w - 4;

    tui_at(p, start_x, current_y++);
    APPEND_LIT(p, "\033[1mRAM\033[22m Total: ");

    append_fixed_shift_2d(p, memmon->total, 20);
    APPEND_LIT(p, " GiB");

    if (memmon->total == 0) return;

    int p_used   = (memmon->used   * 100) / memmon->total;
    int p_avail  = (memmon->available * 100) / memmon->total;
    int p_cached = (memmon->cached * 100) / memmon->total;
    int p_free   = (memmon->free   * 100) / memmon->total;

    draw_meter(p, start_x, current_y++, inner_w, "Used ", TC_MEM_USED,  p_used,   memmon->used);
    draw_meter(p, start_x, current_y++, inner_w, "Avail", TC_MEM_AVAIL, p_avail,  memmon->available);
    draw_meter(p, start_x, current_y++, inner_w, "Cache", TC_MEM_CACHE, p_cached, memmon->cached);
    draw_meter(p, start_x, current_y++, inner_w, "Free ", TC_MEM_FREE,  p_free,   memmon->free);
}
