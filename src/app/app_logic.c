#include "app.h"
#include "app_internal.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"
#include "theme/theme.h"
#include "ui/term.h"
#include "ui/ui.h"
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

void app_update_layout(CpuMon cpu_ptr[static 1],
                       MemMon mem_ptr[static 1],
                       bool cpu_on_ptr[static 1],
                       bool mem_on_ptr[static 1],
                       bool resize_ptr[static 1],
                       bool redraw_ptr[static 1])
{
   size_t w = term_w;
   size_t h = term_h;
   size_t margin = 1;

   if (*cpu_on_ptr && *mem_on_ptr)
   {
      int split_h = h - 7;
      cpu_ptr->rect = (Rect){margin, margin, w, split_h};
      mem_ptr->rect = (Rect){margin, margin + split_h, w, h - split_h};
   }
   else if (*cpu_on_ptr && !*mem_on_ptr)
   {
      cpu_ptr->rect = (Rect){margin, margin, w, h};
   }
   else if (!(*cpu_on_ptr) && *mem_on_ptr)
   {
      mem_ptr->rect = (Rect){margin, margin, w, h};
   }

   if (*cpu_on_ptr)
      calc_cpu_layout(cpu_ptr);

   *resize_ptr = false;
   *redraw_ptr = true;
}

static void handle_mouse(unsigned delay_ptr[static 1],
                         bool redraw_ptr[static 1])
{
   char seq[32];
   int len = 0;
   struct pollfd pfd_drain = {STDIN_FILENO, POLLIN, 0};

   while (poll(&pfd_drain, 1, 0) > 0 && len < 31)
   {
      if (read(STDIN_FILENO, &seq[len], 1) <= 0)
         break;
      len++;
   }
   seq[len] = '\0';

   if (len > 3 && seq[0] == '[' && seq[1] == '<')
   {
      int btn, mx, my;
      char type;

      if (sscanf(seq + 2, "%d;%d;%d%c", &btn, &mx, &my, &type) == 4)
      {

         if (type == 'M' && btn == 0)
         {

            int base_x = term_w - 12;
            int base_y = 1;

            if (my == base_y)
            {

               if (mx >= base_x && mx <= base_x + 2)
               {
                  if (*delay_ptr > 100)
                     *delay_ptr -= 100;
                  else
                     *delay_ptr = 100;
               }

               int num_len = (*delay_ptr >= 1'000) ? 4 : 3;
               int plus_offset = 3 + num_len + 2 + 1;

               if (mx >= base_x + plus_offset && mx <= base_x + plus_offset + 1)
               {
                  if (*delay_ptr < 9'900)
                     *delay_ptr += 100;
               }
               *redraw_ptr = true;
            }
         }
      }
   }
}

static void handle_keyboard(CpuMon cpu_ptr[static 1],
                            MemMon mem_ptr[static 1],
                            bool app_on_ptr[static 1],
                            bool cpu_on_ptr[static 1],
                            bool mem_on_ptr[static 1],
                            bool resize_ptr[static 1],
                            bool redraw_ptr[static 1],
                            char key)
{
   switch (key)
   {
      case 0x03: // CTRL+C
      case 'q':
      case 'Q':
         *app_on_ptr = false;
         break;
      case '1':
         *cpu_on_ptr = !(*cpu_on_ptr);
         app_update_layout(
            cpu_ptr, mem_ptr, cpu_on_ptr, mem_on_ptr, resize_ptr, redraw_ptr);
         break;
      case '2':
         *mem_on_ptr = !(*mem_on_ptr);
         app_update_layout(
            cpu_ptr, mem_ptr, cpu_on_ptr, mem_on_ptr, resize_ptr, redraw_ptr);
         break;
      case 't':
      case 'T':
         toggle_theme();
         *redraw_ptr = true;
         break;
      case 'r':
      case 'R':
         *redraw_ptr = true;
         break;
   }
}

void app_handle_input(CpuMon cpu_ptr[static 1],
                      MemMon mem_ptr[static 1],
                      bool app_on_ptr[static 1],
                      bool cpu_on_ptr[static 1],
                      bool mem_on_ptr[static 1],
                      bool resize_ptr[static 1],
                      bool redraw_ptr[static 1],
                      unsigned delay_ptr[static 1],
                      int timeout_ms)
{
   if (g_signal_quit)
   {
      *app_on_ptr = false;
      return;
   }

   struct pollfd fds[1];
   fds[0].fd = STDIN_FILENO;
   fds[0].events = POLLIN;

   int ret = poll(fds, 1, timeout_ms);

   if (ret > 0 && (fds[0].revents & POLLIN))
   {
      char key;
      if (read(STDIN_FILENO, &key, 1) > 0)
      {
         if (key == 0x1B)
            handle_mouse(delay_ptr, redraw_ptr);
         else
            handle_keyboard(cpu_ptr,
                            mem_ptr,
                            app_on_ptr,
                            cpu_on_ptr,
                            mem_on_ptr,
                            resize_ptr,
                            redraw_ptr,
                            key);
      }
   }
   else if (ret < 0 && g_signal_quit)
   {
      *app_on_ptr = false;
   }
}

void app_update_state(CpuMon cpu_ptr[static 1],
                      MemMon mem_ptr[static 1],
                      bool cpu_on,
                      bool mem_on)
{
   if (cpu_on)
      update_cpu(cpu_ptr);
   if (mem_on)
      update_mem_data(mem_ptr);
}
