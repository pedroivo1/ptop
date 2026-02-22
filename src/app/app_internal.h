#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "app/app.h"
#include <stddef.h>

void app_update_layout(CpuMon cpu_ptr[static 1],
                       MemMon mem_ptr[static 1],
                       bool cpu_on_ptr[static 1],
                       bool mem_on_ptr[static 1],
                       bool resize_ptr[static 1],
                       bool redraw_ptr[static 1]);

void app_handle_input(CpuMon cpu_ptr[static 1],
                      MemMon mem_ptr[static 1],
                      bool app_on_ptr[static 1],
                      bool cpu_on_ptr[static 1],
                      bool mem_on_ptr[static 1],
                      bool resize_ptr[static 1],
                      bool redraw_ptr[static 1],
                      unsigned delay_ptr[static 1],
                      int timeout_ms);

void app_update_state(CpuMon cpu_ptr[static 1],
                      MemMon mem_ptr[static 1],
                      bool cpu_on,
                      bool mem_on);

ptrdiff_t app_draw(CpuMon cpu_ptr[static 1],
                   MemMon mem_ptr[static 1],
                   bool cpu_on_ptr[static 1],
                   bool mem_on_ptr[static 1],
                   bool resize_ptr[static 1],
                   bool redraw_ptr[static 1],
                   unsigned delay,
                   char buf[static 1]);

#endif
