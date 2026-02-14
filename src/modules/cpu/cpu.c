#include <fcntl.h>
#include <stdlib.h>

#include "common/cfg.h"
#include "ui/ui.h"
#include "modules/cpu/cpu_internal.h"

void init_cpu(CpuMon *cpumon)
{
    memset(cpumon, 0, sizeof(*cpumon));

    get_topology(cpumon);

    int num_lines = cpumon->thread_count + 2;
    size_t line_len = 512;
    cpumon->stat_buffer_size = num_lines * line_len;
    cpumon->stat_buffer = malloc(cpumon->stat_buffer_size);

    cpumon->prev_total = calloc(cpumon->thread_count, sizeof(*cpumon->prev_total));
    cpumon->prev_idle = calloc(cpumon->thread_count, sizeof(*cpumon->prev_idle));
    cpumon->usage = calloc(cpumon->thread_count, sizeof(*cpumon->usage));
    cpumon->fd_freq = calloc(cpumon->phy_count, sizeof(*cpumon->fd_freq));

    cpumon->graph_hist = calloc(cpumon->thread_count * GRAPH_WIDTH, sizeof(uint8_t));

    cpumon->fd_stat = open(STAT_PATH, O_RDONLY);

    int hwmon_cpu_id = get_temp_id();
    char path[64];
    char *p = path;
    APPEND_LIT(&p, "/sys/class/hwmon/hwmon");
    append_num(&p, hwmon_cpu_id);
    APPEND_LIT(&p, "/temp");
    append_num(&p, 1);
    APPEND_LIT(&p, "_input");
    *p = '\0';
    cpumon->fd_temp = open(path, O_RDONLY);

    for (size_t i = 0; i < cpumon->phy_count; i++)
    {
        p = path;
        APPEND_LIT(&p, "/sys/devices/system/cpu/cpu");
        append_num(&p, i);
        APPEND_LIT(&p, "/cpufreq/scaling_cur_freq");
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
    int table_w = 32;

    if (table_w * 2 > cpumon->rect.w)
        table_w = cpumon->rect.w / 2;
    
    if (table_w < 18)
        table_w = 11;

    int table_h = cpumon->thread_count + 2;

    int table_x = cpumon->rect.x + cpumon->rect.w - table_w - 1;

    int table_y = cpumon->rect.y + ((cpumon->rect.h - table_h) >> 1);

    cpumon->r_table = (Rect){
        .x = (uint16_t)table_x,
        .y = (uint16_t)table_y,
        .w = (uint16_t)table_w,
        .h = (uint16_t)table_h
    };
}


char *draw_cpu_ui(CpuMon *cpumon, char *p)
{
    Rect r = cpumon->rect;
    Rect rt = cpumon->r_table;

    // --- MAIN BOX ---
    p = tui_draw_box(p, r.x, r.y, r.w, r.h, TC_CPU_BD);

    // --- TABLE BOX ---
    p = tui_draw_box(p, rt.x, rt.y, rt.w, rt.h, TX_DIM1);

    // --- TABLE UI ---
    p = tui_draw_up_space(p, rt.x + 2, rt.y, 4);
    p = tui_draw_up_space(p, rt.x + 9, rt.y, 7);

    APPEND_LIT(&p, TX_FONT);
    p = draw_temp_ui(p, rt.x + 3, rt.y);
    p = draw_freq_ui(p, rt.x + 10, rt.y);
    for (int i = 0; i < cpumon->thread_count; i++)
    {
        int row = rt.y + i + 1;
        p = tui_at(p, rt.x + 1, row);
        p = draw_label_ui(p, i);

        int width = rt.w - 11;
        p = draw_usage_ui(p, width);
    }

    p = draw_uptime_ui(p, r.x + 2, r.y + 1);
    p = draw_load_avg_ui(p, r.x + 2, r.y + r.h - 2);

    return p;
}

char *draw_cpu_data(CpuMon* cpumon, char *p)
{
    Rect rt = cpumon->r_table;

    // --- TABLE METRICS ---
    p = draw_temp_data(p, rt.x + 3, rt.y, cpumon->temp);
    p = draw_freq_data(p, rt.x + 10, rt.y, cpumon->freq);
    for (int i = 0; i < cpumon->thread_count; i++)
    {
        int row = rt.y + i + 1;
        p = tui_at(p, rt.x + 5, row);

        int width = rt.w - 11;

        uint8_t *core_hist_ptr = &cpumon->graph_hist[i * GRAPH_WIDTH];
        p = tui_draw_graph(p, core_hist_ptr, width, cpumon->graph_head);
        p = draw_usage_data(p, cpumon->usage[i]);
    }

    APPEND_LIT(&p, TX_FONT);
    p = draw_uptime_data(p, cpumon->rect.x + 5, cpumon->rect.y + 1, cpumon->uptime);

    p = draw_load_avg_data(p, cpumon->rect.x + 6, cpumon->rect.y + cpumon->rect.h - 2, cpumon->load_avg);

    return p;
}
