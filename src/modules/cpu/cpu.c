#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util/util.h"
#include "ui/ui.h"
#include "ui/term.h"
#include "cfg/path.h"
#include "modules/cpu/cpu_internal.h"

void init_cpu(CpuMon *cpumon)
{
    memset(cpumon, 0, sizeof(*cpumon));
    cpumon->graph_width = 21;

    get_topology(cpumon);

    int num_lines = cpumon->thread_count + 2;
    size_t line_len = 512;
    cpumon->stat_buffer_size = num_lines * line_len;
    cpumon->stat_buffer = malloc(cpumon->stat_buffer_size);

    cpumon->prev_total = calloc(cpumon->thread_count, sizeof(*cpumon->prev_total));
    cpumon->prev_idle = calloc(cpumon->thread_count, sizeof(*cpumon->prev_idle));
    cpumon->usage = calloc(cpumon->thread_count, sizeof(*cpumon->usage));
    cpumon->fd_freq = calloc(cpumon->phy_count, sizeof(*cpumon->fd_freq));

    cpumon->graph_hist = calloc(cpumon->thread_count * cpumon->graph_width, sizeof(uint8_t));

    cpumon->fd_stat = open(STAT, O_RDONLY);

    int hwmon_cpu_id = get_temp_id();
    char path[128];
    char *p = path;
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

void deinit_cpu(CpuMon *cpumon)
{
    if (cpumon->fd_stat > 0) close(cpumon->fd_stat);
    if (cpumon->fd_temp > 0) close(cpumon->fd_temp);
    
    if (cpumon->fd_freq) {
        for (int i = 0; i < cpumon->phy_count; i++) {
            if (cpumon->fd_freq[i] > 0) close(cpumon->fd_freq[i]);
        }
    }

    if (cpumon->stat_buffer) free(cpumon->stat_buffer);
    if (cpumon->fd_freq)     free(cpumon->fd_freq);
    if (cpumon->prev_total)  free(cpumon->prev_total);
    if (cpumon->prev_idle)   free(cpumon->prev_idle);
    if (cpumon->usage)       free(cpumon->usage);
    if (cpumon->graph_hist)  free(cpumon->graph_hist);
}

void update_cpu_data(CpuMon *cpumon)
{
    get_freq_mhz(cpumon);
    get_temp_c(cpumon);
    parse_stats(cpumon);
    get_load_avg(cpumon);
}

void recalc_cpu(CpuMon *cpumon)
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

    // --- ROWS ITERATIONS ---
    rows = (cpumon->thread_count + cols - 1) / cols;

    // --- WIDTH ---
    uint16_t gaps = (cols > 0) ? (cols - 1) : 0;
    uint16_t max_allowed_w = cpumon->rect.w / 2;

    uint16_t table_w = (cols * IDEAL_COL_WIDTH) + gaps + 2;
    if (table_w > max_allowed_w) {
        table_w = max_allowed_w;
    }

    // --- HEIGHT ---
    uint16_t table_h = 0;
    table_h = rows + (BORDER_PADDING*2); 

    // --- RECT ---
    uint16_t table_x = cpumon->rect.x + cpumon->rect.w - table_w - 1;
    uint16_t table_y = cpumon->rect.y + (cpumon->rect.h - table_h) / 2;
    cpumon->r_table = (Rect){
        .x = (uint16_t)table_x,
        .y = (uint16_t)table_y,
        .w = (uint16_t)table_w,
        .h = (uint16_t)table_h
    };

    // --- GRID ---
    cpumon->table_cols = cols;
    cpumon->table_rows = rows;
    if (cols > 0) {
        int available_width = cpumon->r_table.w - 2 - gaps;
        if (available_width < 1) available_width = 1;
        
        cpumon->col_width = available_width / cols;
    } else {
        cpumon->col_width = 1;
    }
}

void draw_cpu_ui(CpuMon *cpumon, char **p)
{
    Rect r = cpumon->rect;
    Rect rt = cpumon->r_table;

    // --- MAIN BOX ---
    tui_draw_box(p, r.x, r.y, r.w, r.h, TC_CPU_BD);

    // --- TABLE BOX ---
    tui_draw_box(p, rt.x, rt.y, rt.w, rt.h, TX_DIM1);
    
    tui_draw_up_space(p, rt.x + 2, rt.y, 4);
    tui_draw_up_space(p, rt.x + 9, rt.y, 7);
    APPEND_LIT(p, TX_FONT);
    draw_temp_ui(p, rt.x + 3, rt.y);
    draw_freq_ui(p, rt.x + 10, rt.y);

    for (int i = 0; i < cpumon->thread_count; i++)
    {
        int col = i / cpumon->table_rows;
        int row = i % cpumon->table_rows;
        
        int bx = rt.x + 1 + (col * cpumon->col_width) + col;
        int by = rt.y + 1 + row;

        tui_at(p, bx, by);

        draw_label_ui(p, i);

        int width = cpumon->col_width - 9;
        if (width < 0) width = 0;
        
        draw_usage_ui(p, width);
    }

    draw_uptime_ui(p, r.x + 2, r.y + 1);
    draw_load_avg_ui(p, r.x + 2, r.y + r.h - 2);
}

void draw_cpu_data(CpuMon* cpumon, char **p)
{
    Rect rt = cpumon->r_table;

    // --- TABLE METRICS ---
    draw_temp_data(p, rt.x + 3, rt.y, cpumon->temp);
    draw_freq_data(p, rt.x + 10, rt.y, cpumon->freq);

    for (int i = 0; i < cpumon->thread_count; i++)
    {
        int col = i / cpumon->table_rows;
        int row = i % cpumon->table_rows;

        int bx = rt.x + 1 + (col * cpumon->col_width) + col;
        int by = rt.y + 1 + row;

        int available_w = cpumon->col_width - 9;
        if (available_w < 0) available_w = 0;

        int draw_w = available_w;
        int padding_left = 0;

        tui_at(p, bx + 4 + padding_left, by);

        uint8_t *core_hist_ptr = &cpumon->graph_hist[i * cpumon->graph_width];

        tui_draw_graph(p, core_hist_ptr, draw_w, cpumon->graph_head);

        draw_usage_data(p, cpumon->usage[i]);
    }

    APPEND_LIT(p, TX_FONT);
    draw_uptime_data(p, cpumon->rect.x + 5, cpumon->rect.y + 1, cpumon->uptime);
    draw_load_avg_data(p, cpumon->rect.x + 6, cpumon->rect.y + cpumon->rect.h - 2, cpumon->load_avg);
}
