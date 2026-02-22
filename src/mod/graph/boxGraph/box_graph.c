#include "mod/graph/boxGraph/box_graph.h"
#include "ui/ui.h"
#include "util/rect.h"
#include <stddef.h>
#include <stdint.h>

void tui_draw_box_graph(char** p,
                        uint8_t* data,
                        size_t capacity,
                        int head,
                        Rect r)
{
   size_t center_y = r.h / 2;
   int is_even_h = (r.h % 2 == 0);

   int max_dist = (r.h - 1) / 2;
   if (!is_even_h)
      max_dist = (r.h / 2) - 1;

   if (max_dist < 1)
      max_dist = 1;

   int total_subpixels = (max_dist + 1) * 8;

   for (size_t y = 0; y < r.h; y++)
   {
      tui_at(p, r.x, r.y + y);

      int dist;
      const char** current_dots;

      if (is_even_h)
      {
         if (y < center_y)
         {
            dist = (center_y - 1) - y;
            current_dots = dots_braille;
         }
         else
         {
            dist = y - center_y;
            current_dots = dots_braille_inv;
         }
      }
      else if (y <= center_y)
      {
         dist = center_y - y;
         current_dots = dots_braille;
      }
      else
      {
         dist = y - center_y - 1;
         current_dots = dots_braille_inv;
      }

      int color_idx = (dist << 3) / max_dist;

      if (color_idx > 6)
         color_idx = 6;

      int row_start_pixel = dist * 8;
      int row_end_pixel = (dist + 1) * 8;

      for (size_t x = 0; x < r.w; x++)
      {
         size_t idx = (head + capacity - r.w + x) % capacity;
         uint8_t val = data[idx];
         int needed_subpixels = (val * total_subpixels) / 100;

         if (needed_subpixels >= row_end_pixel)
         {
            append_str(p, gradient_perc[color_idx]);
            append_str(p, current_dots[7]);
         }
         else if (needed_subpixels > row_start_pixel)
         {
            int remainder = needed_subpixels - row_start_pixel;
            int char_idx = remainder - 1;

            if (char_idx < 0)
               char_idx = 0;
            if (char_idx > 7)
               char_idx = 7;

            append_str(p, gradient_perc[color_idx]);
            append_str(p, current_dots[char_idx]);
         }
         else if (dist == 0)
         {
            append_str(p, gradient_perc[0]);
            append_str(p, current_dots[0]);
         }
         else
         {
            append_str(p, " ");
         }
      }
   }
}
