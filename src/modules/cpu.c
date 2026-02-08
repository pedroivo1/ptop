#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <stdalign.h>
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
        p = APPEND_LIT(p, "/sys/class/hwmon/hwmon");
        p = append_num(p, i);
        p = APPEND_LIT(p, "/name");
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

void init_cpu(CpuMon *cpumon)
{
    memset(cpumon, 0, sizeof(*cpumon));

    cpumon->fd_stat = open(STAT_PATH, O_RDONLY);

    int hwmon_cpu_id = get_coretemp_cpu_id();
    char path[64];
    char *p = path;
    p = APPEND_LIT(p, "/sys/class/hwmon/hwmon");
    p = append_num(p, hwmon_cpu_id);
    p = APPEND_LIT(p, "/temp");
    p = append_num(p, 1);
    p = APPEND_LIT(p, "_input");
    *p = '\0';
    cpumon->fd_temp = open(path, O_RDONLY);

    for (size_t i = 0; i < PHY_CORES_N; i++)
    {
        p = path;
        p = APPEND_LIT(p, "/sys/devices/system/cpu/cpu");
        p = append_num(p, i);
        p = APPEND_LIT(p, "/cpufreq/scaling_cur_freq");
        *p = '\0';
        cpumon->fd_freq[i] = open(path, O_RDONLY);
    }
}

void deinit_cpu(CpuMon *cpumon)
{
    close(cpumon->fd_stat);
    close(cpumon->fd_temp);
    for (int i = 0; i < PHY_CORES_N; i++)
        close(cpumon->fd_freq[i]);
}

static void get_temp_c(CpuMon *cpumon)
{
    cpumon->temp = read_sysfs_uint64(cpumon->fd_temp) / 1000;
}

static void get_freq_mhz(CpuMon *cpumon)
{
    int total = 0;
    for (int i = 0; i < PHY_CORES_N; i++)
        total += read_sysfs_uint64(cpumon->fd_freq[i]);

    cpumon->freq = total / (1000 * PHY_CORES_N);
}

static void parse_stats(CpuMon* cpumon)
{
    char buf[STAT_BUFF_LEN];
    ssize_t bytes_read = pread(cpumon->fd_stat, buf, sizeof(buf) - 1, 0);
    if (bytes_read < 0) return;

    char *p = buf;
    while (*p && *p != '\n') p++;
    if (*p == '\n') p++;
    for (size_t cpu_id = 0; cpu_id < CORES_N; cpu_id++)
    {
        if (p[0] != 'c' || p[1] != 'p' || p[2] != 'u') break;
        p += 4;
        if (*p >= '0' && *p <= '9') p++;
        p++;

        uint64_t active = 0;
        uint64_t total_idle = 0;
        uint64_t val;
        // USER, NICE, SYSTEM
        for (int k=0; k<3; k++)
        {
            val = 0;
            while (*p >= '0') val = (val * 10) + (*p++ - '0');
            active += val;
            while (*p == ' ') p++;
        }

        // IDLE, IOWAIT
        for (int k=0; k<2; k++)
        {
            val = 0;
            while (*p >= '0') val = (val * 10) + (*p++ - '0');
            total_idle += val;
            while (*p == ' ') p++;
        }

        // IRQ, SOFTIRQ, STEAL
        for (int k=0; k<3; k++)
        {
            val = 0;
            while (*p >= '0') val = (val * 10) + (*p++ - '0');
            active += val;
            while (*p == ' ') p++;
        }

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;

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
        cpumon->graph_hist[cpu_id][cpumon->graph_head] = current_usage;
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
static inline char *draw_temp_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, "  ");
    p = APPEND_LIT(p, "°C");
    return p;
}

static inline char *draw_temp_data(char *p, int x, int y, int temp)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, BOLD);
    p = append_str(p, gradient_temp[(temp + 128) >> 4]);
    p = append_num(p, temp);
    p = APPEND_LIT(p, WHITE NOBOLD);
    return p;
}

static inline char *draw_freq_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, "   ");
    p = APPEND_LIT(p, " GHz");
    return p;
}

static inline char *draw_freq_data(char *p, int x, int y, int mhz)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, BOLD);
    p = append_fixed(p, mhz, 1000, 10);
    p = APPEND_LIT(p, NOBOLD);
    return p;
}

static inline char *draw_label_ui(char *p, int id)
{
    p = APPEND_LIT(p, BOLD);
    p = APPEND_LIT(p, "C");
    p = append_num(p, id);
    p = APPEND_LIT(p, NOBOLD);
    *p++ = ' ';
    if (id < 10) *p++ = ' ';
    return p;
}

static inline char *draw_usage_ui(char *p)
{
    for (int i = 0; i < GRAPH_WIDTH; i++)
    {
        p = APPEND_LIT(p, " ");
    }
    p = APPEND_LIT(p, "    ");
    p = APPEND_LIT(p, "%");
    return p;
}

static inline char *draw_usage_data(char *p, int usage)
{
    p = APPEND_LIT(p, " ");
    if (usage)
        p = append_str(p, gradient_perc[(usage >> 4) & 7]);
    else
        p = APPEND_LIT(p, GRAY);
    if (usage < 10) *p++ = ' ';
    if (usage < 100) *p++ = ' ';
    p = append_num(p, usage);
    return p;
}

static inline char *draw_load_avg_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, "AVG: ");

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
        if (k < 2) p = APPEND_LIT(p, "  ");
    }
    p = APPEND_LIT(p, WHITE);

    return p;
}

static inline char *draw_uptime_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    p = APPEND_LIT(p, "up ");
    return p;
}

static inline char *draw_uptime_data(char *p, int x, int y, int uptime)
{
    p = tui_at(p, x, y);
    long up = uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;
    if  (hours < 10) p = APPEND_LIT(p, "0");
    p = append_num(p, hours);
    p = APPEND_LIT(p, ":");
    if (mins < 10) p = APPEND_LIT(p, "0");
    p = append_num(p, mins);
    p = APPEND_LIT(p, ":");
    if (secs < 10) p = APPEND_LIT(p, "0");
    p = append_num(p, secs);
    return p;
}

char *draw_cpu_ui(char *p, int x, int y, int w, int h)
{
    // --- MAIN BOX ---
    p = tui_draw_box(p, x, y, w, h, CPU_BORDER_C);

    // --- TABLE BOX ---
    int table_w = 32;
    int table_x = x + w - table_w - 1;
    int table_h = CORES_N + 2;
    int table_y = y + ((h - table_h) >> 1);
    p = tui_draw_box(p, table_x, table_y, table_w, table_h, GRAY);
    p = tui_draw_up_space(p, table_x + 4, table_y, 4);
    p = tui_draw_up_space(p, table_x + 12, table_y, 7);
    p = tui_draw_bottom_space(p, table_x + 4, table_y + table_h - 1, 21);

    p = APPEND_LIT(p, WHITE);
    p = draw_temp_ui(p, table_x + 5, table_y);
    p = draw_freq_ui(p, table_x + 13, table_y);
    p = draw_load_avg_ui(p, table_x + 5, table_y + table_h - 1);
    for (int i = 0; i < CORES_N; i++)
    {
        int row = table_y + i + 1;
        p = tui_at(p, table_x + 1, row);
        p = draw_label_ui(p, i);
        p = draw_usage_ui(p);
    }

    p = draw_uptime_ui(p, x + 2, y + h - 2);

    return p;
}

char *draw_cpu_data(CpuMon* cpumon, char *p, int x, int y, int w, int h)
{
    // --- TABLE BOX ---
    int table_w = 32;
    int table_x = x + w - table_w - 1;
    int table_h = CORES_N + 2;
    int table_y = y + ((h - table_h) >> 1);

    // --- TABLE METRICS ---
    p = draw_temp_data(p, table_x + 5, table_y, cpumon->temp);
    p = draw_freq_data(p, table_x + 13, table_y, cpumon->freq);
    p = draw_load_avg_data(p, table_x + 10, table_y + table_h - 1, cpumon->load_avg);
    for (int i = 0; i < CORES_N; i++)
    {
        int row = table_y + i + 1;
        p = tui_at(p, table_x + 5, row);
        p = tui_draw_graph(p, cpumon->graph_hist[i], GRAPH_WIDTH, cpumon->graph_head);
        p = draw_usage_data(p, cpumon->usage[i]);
    }

    p = APPEND_LIT(p, WHITE);
    p = draw_uptime_data(p, x + 5, y + h - 2, cpumon->uptime);

    return p;
}
