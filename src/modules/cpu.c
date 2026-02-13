#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <stdalign.h>
#include <stdlib.h>
#include "cpu.h"
#include "../utils.h"
#include "../tui.h"

int get_coretemp_cpu_id()
{
    char path[32];
    char buf[32];
    char *p;

    for (size_t i = 0; i < HWMON_N; i++)
    {
        p = path;
        APPEND_LIT(&p, "/sys/class/hwmon/hwmon");
        append_num(&p, i);
        APPEND_LIT(&p, "/name");
        *p = '\0';
        int fd = open(path, O_RDONLY);
        ssize_t bytes_read = read(fd, buf, sizeof(buf));
        close(fd);
        if (bytes_read == -1) return -1;

        p = buf;
        int j = 0;
        while (*p && j < CORE_LABEL_NAME_N)
        {
            if (*p++ != CORE_LABEL_NAME[j++])
                break;
        }
        if (j == CORE_LABEL_NAME_N) return i;
    }
    return -1;
}
static inline void detect_cpu_topology(CpuMon *cpumon)
{
    cpumon->phy_count = 1;
    cpumon->thread_count = 1;
    cpumon->threads_per_core = 1;

    int total_threads = get_nprocs(); 
    if (total_threads <= 0) total_threads = 1;

    int fd = open("/proc/cpuinfo", O_RDONLY);
    if (fd < 0) {
        cpumon->thread_count = (uint16_t)total_threads;
        cpumon->phy_count = (uint16_t)total_threads;
        return;
    }

    char buf[1024]; 
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytes_read <= 0) {
        cpumon->thread_count = (uint16_t)total_threads;
        return;
    }

    buf[bytes_read] = '\0';

    int siblings = 0;
    int phy_cores = 0;

    char *p = buf;
    char *line;

    while (*p)
    {
        line = p;
        while (*p && *p != '\n') p++;
        
        if (*p == '\n') *p++ = '\0';

        if (strncmp(line, "siblings", 8) == 0) {
            char *val = strchr(line, ':');
            if (val) siblings = atoi(val + 1);
        }
        
        if (strncmp(line, "cpu cores", 9) == 0) {
            char *val = strchr(line, ':');
            if (val) phy_cores = atoi(val + 1);
        }

        if (siblings > 0 && phy_cores > 0) break;
    }

    if (siblings <= 0) siblings = total_threads;
    if (phy_cores <= 0) phy_cores = siblings;

    cpumon->thread_count = (uint16_t)total_threads;
    cpumon->phy_count    = (uint16_t)phy_cores;

    if (phy_cores > 0)
        cpumon->threads_per_core = (uint16_t)(siblings / phy_cores);
    else
        cpumon->threads_per_core = 1;
}
void init_cpu(CpuMon *cpumon)
{
    memset(cpumon, 0, sizeof(*cpumon));

    detect_cpu_topology(cpumon);

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

    int hwmon_cpu_id = get_coretemp_cpu_id();
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

static void get_temp_c(CpuMon *cpumon)
{
    cpumon->temp = read_sysfs_uint64(cpumon->fd_temp) / 1000;
}


static void get_freq_mhz(CpuMon *cpumon)
{
    uint64_t total = 0;
    int valid_readings = 0;

    for (int i = 0; i < cpumon->phy_count; i++)
    {
        if (cpumon->fd_freq[i] > 0) {
            uint64_t val = read_sysfs_uint64(cpumon->fd_freq[i]);
            if (val > 0) {
                total += val;
                valid_readings++;
            }
        }
    }

    if (valid_readings > 0)
        cpumon->freq = total / (1000 * valid_readings);
    else
        cpumon->freq = 0;
}

static void parse_stats(CpuMon* cpumon)
{
    ssize_t bytes_read = pread(cpumon->fd_stat, cpumon->stat_buffer, cpumon->stat_buffer_size - 1, 0);
    if (bytes_read < 0) return;

    cpumon->stat_buffer[bytes_read] = '\0';

    char *p = cpumon->stat_buffer;
    
    skip_line(&p);

    for (size_t cpu_id = 0; cpu_id < cpumon->thread_count; cpu_id++)
    {
        if (strncmp(p, "cpu", 3) != 0) break;

        skip_to_digit(&p);
        str_to_uint64(&p);

        uint64_t active = 0;
        uint64_t total_idle = 0;

        // USER, NICE, SYSTEM
        for (int k=0; k<3; k++)
        {
            skip_to_digit(&p);
            active += str_to_uint64(&p);
        }

        // IDLE, IOWAIT
        for (int k=0; k<2; k++)
        {
            skip_to_digit(&p);
            total_idle += str_to_uint64(&p);
        }

        // IRQ, SOFTIRQ, STEAL
        for (int k=0; k<3; k++)
        {
            skip_to_digit(&p);
            active += str_to_uint64(&p);
        }

        skip_line(&p);

        uint64_t total = active + total_idle;
        uint64_t diff_total = total - cpumon->prev_total[cpu_id];
        uint64_t diff_idle  = total_idle - cpumon->prev_idle[cpu_id];

        uint8_t current_usage = 0;
        if (diff_total > 0)
        {
            current_usage = (uint8_t)((diff_total - diff_idle) * 100 / diff_total);
        }

        cpumon->usage[cpu_id] = current_usage;
        cpumon->prev_total[cpu_id] = total;
        cpumon->prev_idle[cpu_id] = total_idle;
        
        int idx = (cpu_id * GRAPH_WIDTH) + cpumon->graph_head;
        cpumon->graph_hist[idx] = current_usage;
    }
    cpumon->graph_head = (cpumon->graph_head + 1) % GRAPH_WIDTH;
}

static void get_load_avg(CpuMon *cpumon)
{
    struct sysinfo si;
    if (sysinfo(&si) == 0)
    {
        cpumon->load_avg[0] = si.loads[0];
        cpumon->load_avg[1] = si.loads[1];
        cpumon->load_avg[2] = si.loads[2];
        cpumon->uptime = si.uptime;
    }
}

void update_cpu_data(CpuMon *cpumon)
{
    get_freq_mhz(cpumon);
    get_temp_c(cpumon);
    parse_stats(cpumon);
    get_load_avg(cpumon);
}


// =============================================================================
// ================================== DISPLAY ==================================
// =============================================================================
void cpu_recalc(CpuMon *cpumon)
{
    int table_w = 32;
    if (table_w * 2 > cpumon->rect.w)
        table_w = cpumon->rect.w/2;
    if (table_w < 32 - 21 + 7)
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

static inline char *draw_temp_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "  ");
    APPEND_LIT(&p, "°C");
    return p;
}

static inline char *draw_temp_data(char *p, int x, int y, int temp)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, BOLD);
    p = append_str(p, gradient_temp[(temp + 128) >> 4]);
    append_num(&p, temp);
    APPEND_LIT(&p, WHITE NOBOLD);
    return p;
}

static inline char *draw_freq_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "   ");
    APPEND_LIT(&p, " GHz");
    return p;
}

static inline char *draw_freq_data(char *p, int x, int y, int mhz)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, BOLD);
    p = append_fixed(p, mhz, 1000, 10);
    APPEND_LIT(&p, NOBOLD);
    return p;
}

static inline char *draw_label_ui(char *p, int id)
{
    APPEND_LIT(&p, BOLD);
    APPEND_LIT(&p, "C");
    append_num(&p, id);
    APPEND_LIT(&p, NOBOLD);
    *p++ = ' ';
    if (id < 10) *p++ = ' ';
    return p;
}

static inline char *draw_usage_ui(char *p, int width)
{
    for (int i = 0; i < width; i++)
    {
        APPEND_LIT(&p, " ");
    }
    APPEND_LIT(&p, "    ");
    APPEND_LIT(&p, "%");
    return p;
}

static inline char *draw_usage_data(char *p, int usage)
{
    APPEND_LIT(&p, " ");
    if (usage)
        p = append_str(p, gradient_perc[(usage >> 4) & 7]);
    else
        APPEND_LIT(&p, GRAY);
    if (usage < 10) *p++ = ' ';
    if (usage < 100) *p++ = ' ';
    append_num(&p, usage);
    return p;
}

static inline char *draw_load_avg_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "AVG ");

    return p;
}

static inline char *draw_load_avg_data(char *p, int x, int y, uint32_t avg[3])
{
    p = tui_at(p, x, y);
    for (int k = 0; k < 3; k++)
    {
        uint32_t raw = avg[k];
        uint32_t l_idx = raw >> 16;
        if (l_idx > 7) l_idx = 7;

        p = append_str(p, gradient_perc[l_idx]);
        p = append_fixed(p, raw, 65536, 100);
        if (k < 2) APPEND_LIT(&p, " ");
    }
    APPEND_LIT(&p, WHITE);

    return p;
}

static inline char *draw_uptime_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "up ");
    return p;
}

static inline char *draw_uptime_data(char *p, int x, int y, int uptime)
{
    p = tui_at(p, x, y);
    long up = uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;
    if  (hours < 10) APPEND_LIT(&p, "0");
    append_num(&p, hours);
    APPEND_LIT(&p, ":");
    if (mins < 10) APPEND_LIT(&p, "0");
    append_num(&p, mins);
    APPEND_LIT(&p, ":");
    if (secs < 10) APPEND_LIT(&p, "0");
    append_num(&p, secs);
    return p;
}

char *draw_cpu_ui(CpuMon *cpumon, char *p)
{
    Rect r = cpumon->rect;
    Rect rt = cpumon->r_table;

    // --- MAIN BOX ---
    p = tui_draw_box(p, r.x, r.y, r.w, r.h, CPU_BORDER_C);

    // --- TABLE BOX ---
    p = tui_draw_box(p, rt.x, rt.y, rt.w, rt.h, GRAY);

    // --- TABLE UI ---
    p = tui_draw_up_space(p, rt.x + 2, rt.y, 4);
    p = tui_draw_up_space(p, rt.x + 9, rt.y, 7);

    APPEND_LIT(&p, WHITE);
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

    APPEND_LIT(&p, WHITE);
    p = draw_uptime_data(p, cpumon->rect.x + 5, cpumon->rect.y + 1, cpumon->uptime);

    p = draw_load_avg_data(p, cpumon->rect.x + 6, cpumon->rect.y + cpumon->rect.h - 2, cpumon->load_avg);

    return p;
}
