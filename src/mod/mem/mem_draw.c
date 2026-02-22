#include "theme/theme.h"
#include "ui/ui.h"
#include "util/util.h"
#include <stdint.h>

void draw_meter(char* p[static 1],
                size_t x,
                size_t y,
                size_t w,
                const char* label,
                const char* color_bar,
                unsigned perc,
                uint64_t val_kb)
{
   size_t label_w = 12;
   size_t value_w = 9;
   size_t bar_w = 5;
   if (w > label_w + value_w + 5)
      bar_w = w - label_w - value_w;

   tui_at(p, x, y);

   append_str(p, theme.fg);
   append_str(p, label);
   *(*p)++ = ':';

   if (perc < 100)
   {
      *(*p)++ = ' ';
      if (perc < 10)
         *(*p)++ = ' ';
   }

   append_str(p, color_bar);
   append_num(p, perc);
   *(*p)++ = '%';

   size_t fill = (bar_w * perc) / 100;
   for (size_t i = 0; i < fill; i++)
      *(*p)++ = '|';
   append_str(p, theme.dim_dark);
   for (size_t i = fill; i < bar_w; i++)
      *(*p)++ = '|';

   append_str(p, theme.fg);
   *(*p)++ = ' ';
   append_fixed_shift_2d(p, val_kb, 20);
   if (val_kb < 10'485'760)
      *(*p)++ = ' ';
   APPEND_LIT(p, " GiB");
}
