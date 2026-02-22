#include "app/app.h"
#include "app/app_internal.h"
#include "cfg/buf.h"
#include "mod/cpu/cpu.h"
#include "mod/mem/mem.h"
#include "theme/theme.h"
#include "ui/term.h"
#include "ui/ui.h"
#include "util/util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void app_init(CpuMon cpu[static 1], MemMon mem[static 1])
{
   tui_setup(theme.bg, theme.fg);
   set_theme(THEME_DARK);

   init_cpu(cpu);
   init_mem(mem);
}

void app_run(CpuMon cpu[static 1],
             MemMon mem[static 1],
             unsigned ptr_delay[static 1])
{
   bool app_on = true;
   bool cpu_on = true;
   bool mem_on = false;
   bool resize = true;
   bool redraw = true;

   app_update_layout(cpu, mem, &cpu_on, &mem_on, &resize, &redraw);

   char buf[WRITE_BUF_LEN];
   unsigned last_update_time = 0;
   while (app_on)
   {
      if (g_signal_quit)
         app_on = false;

      unsigned now = (unsigned)current_time_ms();

      int is_tick =
         (last_update_time == 0 || now - last_update_time >= *ptr_delay);

      if (is_tick)
      {
         last_update_time = now;
         app_update_state(cpu, mem, cpu_on, mem_on);
      }

      if (is_tick || resize || redraw)
      {
         ptrdiff_t bytes_written = app_draw(
            cpu, mem, &cpu_on, &mem_on, &resize, &redraw, *ptr_delay, buf);

         if (bytes_written > 0)
         {
            if (write(STDOUT_FILENO, buf, bytes_written) == -1)
               break;
         }
      }

      uint64_t time_spent = current_time_ms() - last_update_time;
      int time_to_wait = *ptr_delay - time_spent;
      if (time_to_wait < 0)
         time_to_wait = 0;

      app_handle_input(cpu,
                       mem,
                       &app_on,
                       &cpu_on,
                       &mem_on,
                       &resize,
                       &redraw,
                       ptr_delay,
                       time_to_wait);
   }
}

void app_destroy(CpuMon cpu[static 1], MemMon mem[static 1])
{
   deinit_cpu(cpu);
   deinit_mem(mem);

   tui_restore();
}
