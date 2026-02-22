#include "app.h"
#include "app_internal.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"
#include "theme/theme.h"
#include "ui/term.h"
#include "ui/ui.h"
#include "util/util.h"
#include <stddef.h>

void draw_delay_ctl(unsigned delay, char* buf_ptr[static 1])
{

   size_t x = 0;
   if (term_w > 12)
      x = term_w - 12;
   size_t y = 1;

   tui_at(buf_ptr, x, y);

   append_str(buf_ptr, theme.cpu_bd);
   APPEND_LIT(buf_ptr, BOX_TR);
   append_str(buf_ptr, theme.fg);
   APPEND_LIT(buf_ptr, "- ");

   append_num(buf_ptr, delay);
   APPEND_LIT(buf_ptr, "ms");

   APPEND_LIT(buf_ptr, " +");
   append_str(buf_ptr, theme.cpu_bd);
   APPEND_LIT(buf_ptr, BOX_TL);
   append_str(buf_ptr, theme.fg);
}

ptrdiff_t app_draw(CpuMon cpu_ptr[static 1],
                   MemMon mem_ptr[static 1],
                   bool cpu_on_ptr[static 1],
                   bool mem_on_ptr[static 1],
                   bool resize_ptr[static 1],
                   bool redraw_ptr[static 1],
                   unsigned delay,
                   char buf[static 1])
{
   char* char_ptr = buf;
   bool physical_resize = false;

   tui_begin_frame(&physical_resize);

   if (physical_resize)
   {
      *resize_ptr = true;
      app_update_layout(
         cpu_ptr, mem_ptr, cpu_on_ptr, mem_on_ptr, resize_ptr, redraw_ptr);
   }

   bool draw_static = *redraw_ptr || *resize_ptr;

   if (draw_static)
   {
      append_str(&char_ptr, theme.bg);
      APPEND_LIT(&char_ptr, "\033[2J");
   }

   if (*cpu_on_ptr)
   {
      if (draw_static)
         draw_cpu_ui(cpu_ptr, &char_ptr);

      draw_cpu_data(cpu_ptr, &char_ptr);
   }

   if (*mem_on_ptr)
   {
      if (draw_static)
         draw_mem_ui(mem_ptr, &char_ptr);

      draw_mem_data(mem_ptr, &char_ptr);
   }

   if (draw_static)
      draw_delay_ctl(delay, &char_ptr);

   *redraw_ptr = false;
   *resize_ptr = false;

   return char_ptr - buf;
}
