#include "monitor.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sysinfo.h>

static inline int read_sysfs_int(int fd)
{
    char buf[16];
    int val = 0;
    ssize_t bytes_read = pread(fd, buf, sizeof(buf) - 1, 0);

    if (bytes_read > 0)
    {
        char *p = buf;
        while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
    }
    return val;
}

void init_cpumon(CpuMonitor *cpumon)
{
    memset(cpumon, 0, sizeof(*cpumon));

    cpumon->fd_stat = open(STAT_PATH, O_RDONLY);

    int hwmon_cpu_id = get_coretemp_id();
    char path[64];
    char *p = path;
    p = append_str(p, "/sys/class/hwmon/hwmon");
    p = append_int(p, hwmon_cpu_id);
    p = append_str(p, "/temp");
    p = append_int(p, 1);
    p = append_str(p, "_input");
    *p = '\0';
    cpumon->fd_temp = open(path, O_RDONLY);

    for (size_t i = 0; i < PHY_CORES_N; i++)
    {
        p = path;
        p = append_str(p, "/sys/devices/system/cpu/cpu");
        p = append_int(p, i);
        p = append_str(p, "/cpufreq/scaling_cur_freq");
        *p = '\0';
        cpumon->fd_freq[i] = open(path, O_RDONLY);
    }
}

void cleanup_cpumon(CpuMonitor *cpumon)
{
    close(cpumon->fd_stat);
    close(cpumon->fd_temp);
    for (int i = 0; i < PHY_CORES_N; i++)
        close(cpumon->fd_freq[i]);        
}

int get_coretemp_id()
{
    char path[32];
    char buf[32];
    char *p;

    for (size_t i = 0; i < HWMON_N; i++)
    {
        p = path;
        p = append_str(p, "/sys/class/hwmon/hwmon");
        p = append_int(p, i);
        p = append_str(p, "/name");
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

static void get_core_temp_c(CpuMonitor *cpumon)
{
    cpumon->temp = read_sysfs_int(cpumon->fd_temp) / 1000;
}

static void get_core_freq_mhz(CpuMonitor *cpumon)
{
    int total = 0;
    for (int i = 0; i < PHY_CORES_N; i++)
        total += read_sysfs_int(cpumon->fd_freq[i]);

    cpumon->freq = total / (1000 * PHY_CORES_N);
}

static void parse_cpu_stats(CpuMonitor* cpumon) {
    static char buf[STAT_BUFF_LEN] __attribute__((aligned(64)));
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
        for (int k=0; k<3; k++) {
            val = 0;
            while (*p >= '0') val = (val * 10) + (*p++ - '0');
            active += val;
            while (*p == ' ') p++;
        }

        // IDLE, IOWAIT
        for (int k=0; k<2; k++) {
            val = 0;
            while (*p >= '0') val = (val * 10) + (*p++ - '0');
            total_idle += val;
            while (*p == ' ') p++;
        }

        // IRQ, SOFTIRQ, STEAL
        for (int k=0; k<3; k++) {
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
        if (diff_total > 0) {
            current_usage = (uint8_t)((diff_total - diff_idle) * 100 / diff_total);
        }

        cpumon->usage[cpu_id] = current_usage;
        cpumon->prev_total[cpu_id] = total;
        cpumon->prev_idle[cpu_id] = total_idle;
        cpumon->graph_hist[cpu_id][cpumon->graph_head] = current_usage;
    }
    cpumon->graph_head = (cpumon->graph_head + 1) % GRAPH_WIDTH;
}

static void get_system_load(CpuMonitor *cpumon)
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

void update_metrics(CpuMonitor *cpumon)
{
    get_core_freq_mhz(cpumon);
    get_core_temp_c(cpumon);
    parse_cpu_stats(cpumon);
    get_system_load(cpumon);
}
