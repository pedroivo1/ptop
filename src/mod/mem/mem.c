#include "cfg/path.h"
#include "mod/mem/mem_internal.h"
#include "ui/term.h"
#include "ui/ui.h"
#include "util/util.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

void init_mem(MemMon memmon[static 1])
{
   memset(memmon, 0, sizeof(*memmon));
   memmon->fd_mem = open(MEMINFO, O_RDONLY);
}

void deinit_mem(MemMon memmon[static 1])
{
   close(memmon->fd_mem);
}

void update_mem_data(MemMon memmon[static 1])
{
   parse_mem(memmon);
}

void draw_mem_ui(MemMon memmon[static 1], char* p[static 1])
{
   size_t x = memmon->rect.x;
   size_t y = memmon->rect.y;
   size_t w = memmon->rect.w;
   size_t h = memmon->rect.h;

   // --- MAIN BOX ---
   tui_draw_box(p, x, y, w, h, theme.mem_bd);

   tui_at(p, memmon->rect.x + 2, memmon->rect.y + 1);
   append_str(p, theme.fg);
   APPEND_LIT(p, "\033[1mRAM\033[22m Total: ");

   append_fixed_shift_2d(p, memmon->total, 20);
   APPEND_LIT(p, " GiB");
}

void draw_mem_data(MemMon memmon[static 1], char* p[static 1])
{
   size_t x = memmon->rect.x;
   size_t y = memmon->rect.y;
   size_t w = memmon->rect.w;

   size_t start_x = x + 2;
   size_t current_y = y + 2;
   size_t inner_w = (w > 4) ? w - 4 : 0;

   if (memmon->total == 0)
      return;

   unsigned p_used = (memmon->used * 100) / memmon->total;
   unsigned p_avail = (memmon->available * 100) / memmon->total;
   unsigned p_cached = (memmon->cached * 100) / memmon->total;
   unsigned p_free = (memmon->free * 100) / memmon->total;

   draw_meter(p, start_x, current_y++, inner_w, "Used ", theme.pct[6], p_used, memmon->used);
   draw_meter(p, start_x, current_y++, inner_w, "Avail", theme.pct[0], p_avail, memmon->available);
   draw_meter(p, start_x, current_y++, inner_w, "Cache", theme.pct[4], p_cached, memmon->cached);
   draw_meter(p, start_x, current_y++, inner_w, "Free ", theme.mem_free, p_free, memmon->free);
}
