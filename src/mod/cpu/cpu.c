#include "cfg/path.h"
#include "mod/cpu/cpu_internal.h"
#include "ui/term.h"
#include "ui/ui.h"
#include "util/util.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void init_cpu(CpuMon cpumon[static 1])
{
   memset(cpumon, 0, sizeof(*cpumon));
   cpumon->graph_width = 21;

   get_topology(cpumon);

   size_t num_lines = cpumon->thread_count + 2;
   size_t line_len = 512;
   cpumon->stat_buffer_size = num_lines * line_len;
   cpumon->stat_buffer = malloc(cpumon->stat_buffer_size);

   cpumon->prev_total = calloc(cpumon->thread_count, sizeof(*cpumon->prev_total));
   cpumon->prev_idle = calloc(cpumon->thread_count, sizeof(*cpumon->prev_idle));
   cpumon->usage = calloc(cpumon->thread_count, sizeof(*cpumon->usage));
   cpumon->fd_freq = calloc(cpumon->phy_count, sizeof(*cpumon->fd_freq));

   cpumon->graph_hist = calloc(cpumon->thread_count * cpumon->graph_width, sizeof(uint8_t));

   cpumon->fd_stat = open(STAT, O_RDONLY);

   uint64_t hwmon_cpu_id;
   do
   {
      hwmon_cpu_id = get_temp_id();
   } while (hwmon_cpu_id == UINT64_MAX);

   char path[128];
   char* p = path;
   APPEND_LIT(&p, HWMON_DIR HWMON);
   append_num(&p, hwmon_cpu_id);
   APPEND_LIT(&p, HWMON_TEMP);
   append_num(&p, 1);
   APPEND_LIT(&p, HWMON_INPUT);
   *p = '\0';
   cpumon->fd_temp = open(path, O_RDONLY);

   for (size_t i = 0; i < cpumon->phy_count; i++)
   {
      p = path;
      APPEND_LIT(&p, SYS_CPU_BASE);
      append_num(&p, i);
      APPEND_LIT(&p, CPU_FREQ_FILE);
      *p = '\0';
      cpumon->fd_freq[i] = open(path, O_RDONLY);
   }
}

void deinit_cpu(CpuMon cpumon[static 1])
{
   if (cpumon->fd_stat > 0)
      close(cpumon->fd_stat);
   if (cpumon->fd_temp > 0)
      close(cpumon->fd_temp);

   if (cpumon->fd_freq)
   {
      for (size_t i = 0; i < cpumon->phy_count; i++)
         if (cpumon->fd_freq[i] > 0)
            close(cpumon->fd_freq[i]);
   }

   if (cpumon->stat_buffer)
      free(cpumon->stat_buffer);
   if (cpumon->fd_freq)
      free(cpumon->fd_freq);
   if (cpumon->prev_total)
      free(cpumon->prev_total);
   if (cpumon->prev_idle)
      free(cpumon->prev_idle);
   if (cpumon->usage)
      free(cpumon->usage);
   if (cpumon->graph_hist)
      free(cpumon->graph_hist);
}

void update_cpu_data(CpuMon cpumon[static 1])
{
   get_freq_mhz(cpumon);
   get_temp_c(cpumon);
   parse_stats(cpumon);
   get_load_avg(cpumon);
}

void recalc_cpu(CpuMon cpumon[static 1])
{
   const uint16_t BORDER_PADDING = 1;

   const uint16_t LABEL_LEN = 4;
   const uint16_t USAGE_LEN = 5;

   const uint16_t IDEAL_COL_WIDTH = LABEL_LEN + cpumon->graph_width + USAGE_LEN;

   // --- ROWS ---
   uint16_t rows = cpumon->rect.h - (BORDER_PADDING * 4);
   if (rows > cpumon->thread_count)
      rows = cpumon->thread_count + BORDER_PADDING;
   if (rows < 1)
      rows = 1;

   // --- COLUMNS ---
   uint16_t cols = (cpumon->thread_count + rows - 1) / rows;
   rows = (cpumon->thread_count + cols - 1) / cols;

   // --- WIDTH ---
   uint16_t gaps = (cols > 0) ? cols - 1 : 0;
   uint16_t max_allowed_w = cpumon->rect.w / 2;

   size_t available_content_w = 1;
   if (max_allowed_w > 2 + gaps)
      available_content_w = max_allowed_w - 2 - gaps;

   uint16_t candidate_col_w = available_content_w / cols;

   if (candidate_col_w > IDEAL_COL_WIDTH)
      candidate_col_w = IDEAL_COL_WIDTH;

   if (candidate_col_w < 16)
   {
      candidate_col_w = 9;
      cpumon->is_compact = true;
   }
   else
   {
      cpumon->is_compact = false;
   }

   uint16_t table_w = (cols * candidate_col_w) + gaps + 2;

   // --- HEIGHT ---
   uint16_t table_h = rows + (BORDER_PADDING * 2);

   // --- POSITION (X, Y)---
   uint16_t table_x = cpumon->rect.x + cpumon->rect.w - table_w - 1;
   uint16_t table_y = cpumon->rect.y + (cpumon->rect.h - table_h) / 2;

   // --- RECT ---
   cpumon->r_table = (Rect){.x = table_x, .y = table_y, .w = table_w, .h = table_h};

   // --- GRID ---
   cpumon->table_cols = cols;
   cpumon->table_rows = rows;
   cpumon->col_width = candidate_col_w;

   // --- MAIN GRAPH ---
   cpumon->r_graph = (Rect){.x = cpumon->rect.x + BORDER_PADDING * 2,
                            .y = cpumon->rect.y + BORDER_PADDING * 2,
                            .w = cpumon->rect.w - table_w - BORDER_PADDING * 4,
                            .h = cpumon->rect.h - BORDER_PADDING * 4};
}

void draw_cpu_ui(CpuMon cpumon[static 1], char* p[static 1])
{
   Rect r = cpumon->rect;
   Rect rt = cpumon->r_table;

   // --- MAIN BOX ---
   tui_draw_box(p, r.x, r.y, r.w, r.h, theme.cpu_bd);

   // --- TABLE BOX ---
   tui_draw_box(p, rt.x, rt.y, rt.w, rt.h, theme.dim);
   draw_temp_ui(p, rt.x + 2, rt.y);
   draw_freq_ui(p, rt.x + 9, rt.y);
   append_str(p, theme.fg);

   for (size_t i = 0; i < cpumon->thread_count; i++)
   {
      unsigned col = i / cpumon->table_rows;
      unsigned row = i % cpumon->table_rows;

      size_t bx = rt.x + 1 + (col * cpumon->col_width) + col;
      size_t by = rt.y + 1 + row;

      // --- DESENHA O SEPARADOR ---
      if (col > 0)
      {
         if (cpumon->is_compact)
            tui_at(p, bx - 2, by);
         else
            tui_at(p, bx - 1, by);
         append_str(p, theme.dim);
         APPEND_LIT(p, BOX_V);
         append_str(p, theme.fg);
      }

      tui_at(p, bx, by);
      draw_label_ui(p, i);

      if (cpumon->is_compact)
         tui_at(p, bx + 3, by);

      size_t width = 0;
      if (cpumon->col_width > 9)
         width = cpumon->col_width - 9;
      draw_usage_ui(p, width);
   }

   draw_uptime_ui(p, r.x + 2, r.y + 1);
   draw_load_avg_ui(p, r.x + 2, r.y + r.h - 2);
}

void draw_cpu_data(CpuMon cpumon[static 1], char* p[static 1])
{
   Rect rt = cpumon->r_table;

   // --- TABLE METRICS ---
   draw_temp_data(p, rt.x + 3, rt.y, cpumon->temp);
   draw_freq_data(p, rt.x + 10, rt.y, cpumon->freq);

   for (size_t i = 0; i < cpumon->thread_count; i++)
   {
      unsigned col = i / cpumon->table_rows;
      unsigned row = i % cpumon->table_rows;

      size_t bx = rt.x + 1 + (col * cpumon->col_width) + col;
      size_t by = rt.y + 1 + row;

      size_t available_w = 0;
      if (cpumon->col_width > 9)
         available_w = cpumon->col_width - 9;

      size_t draw_w = available_w;
      size_t padding_left = 0;

      size_t label_offset = cpumon->is_compact ? 3 : 4;
      tui_at(p, bx + label_offset + padding_left, by);

      uint8_t* core_hist_ptr = &cpumon->graph_hist[i * cpumon->graph_width];
      tui_draw_graph(p, core_hist_ptr, draw_w, cpumon->graph_width, cpumon->graph_head);

      draw_usage_data(p, cpumon->usage[i]);
   }

   append_str(p, theme.fg);
   draw_uptime_data(p, cpumon->rect.x + 5, cpumon->rect.y + 1, cpumon->uptime);
   draw_load_avg_data(p, cpumon->rect.x + 6, cpumon->rect.y + cpumon->rect.h - 2, cpumon->load_avg);

   tui_at(p, cpumon->r_graph.x, cpumon->r_graph.y + cpumon->r_graph.h / 2);
   tui_draw_graph_mirrored(p, cpumon->main_usage, 256, cpumon->main_graph_head, cpumon->r_graph);
}
