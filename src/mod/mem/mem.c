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
    tui_draw_box(p, x, y, w, h, theme.mem_bd);
    append_str(p, theme.fg);
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

    draw_meter(p, start_x, current_y++, inner_w, "Used ", theme.pct[6],  p_used,   memmon->used);
    draw_meter(p, start_x, current_y++, inner_w, "Avail", theme.pct[0], p_avail,  memmon->available);
    draw_meter(p, start_x, current_y++, inner_w, "Cache", theme.pct[4], p_cached, memmon->cached);
    draw_meter(p, start_x, current_y++, inner_w, "Free ", theme.mem_free,  p_free,   memmon->free);
}
