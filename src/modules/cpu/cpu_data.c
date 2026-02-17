#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/sysinfo.h>

#include "modules/cpu/cpu.h"
#include "util/util.h"
#include "cfg/path.h"

int get_temp_id()
{
    DIR *dir = opendir(HWMON_DIR);
    if (!dir)
        return -1;

    struct dirent *entry;
    char path[256];
    char buf[64];
    int found_id = -1;
    char *p;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.' || strncmp(entry->d_name, HWMON, sizeof(HWMON) - 1) != 0)
            continue;

        p = path;
        APPEND_LIT(&p, HWMON_DIR);
        append_str(&p, entry->d_name);
        APPEND_LIT(&p, HWMON_PER);
        *p = '\0'; 

        int fd = open(path, O_RDONLY);
        if (fd < 0)
            continue;

        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n > 0)
        {
            buf[n] = '\0';
            if (buf[n-1] == '\n') buf[n-1] = '\0';

            if (strcmp(buf, CORETEMP) == 0)
            {
                char *p_num = entry->d_name;
                skip_to_digit(&p_num);
                found_id = (int)str_to_uint64(&p_num);
                break;
            }
        }
    }

    closedir(dir);
    return found_id;
}

void get_topology(CpuMon *cpumon)
{
    int total_threads = get_nprocs(); 
    if (total_threads <= 0)
        total_threads = 1;

    cpumon->thread_count = (uint16_t)total_threads;
    
    cpumon->threads_per_core = 1;
    cpumon->phy_count = cpumon->thread_count;

    int fd = open(CPUINFO, O_RDONLY);
    if (fd < 0)
        return;

    char buf[2048]; 
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytes_read <= 0)
        return;

    buf[bytes_read] = '\0';

    int siblings = 0;
    int phy_cores = 0;
    char *p = buf;

    while (*p)
    {
        if (*p == 's' && strncmp(p, THREADS, sizeof(THREADS) - 1) == 0)
        {
            skip_to_digit(&p);
            siblings = (int)str_to_uint64(&p);
        }
        else if (*p == 'c' && strncmp(p, CORES, sizeof(CORES) - 1) == 0)
        {
            skip_to_digit(&p);
            phy_cores = (int)str_to_uint64(&p);
        }
        else
        {
            skip_line(&p);
            continue;
        }

        if (siblings > 0 && phy_cores > 0)
            break;
        skip_line(&p);
    }

    int threads_per_core = 1;

    if (siblings > 0 && phy_cores > 0)
    {
        threads_per_core = siblings / phy_cores;
        if (threads_per_core < 1)
            threads_per_core = 1;
    }

    cpumon->threads_per_core = (uint16_t)threads_per_core;
    cpumon->phy_count = cpumon->thread_count / cpumon->threads_per_core;
}

void get_temp_c(CpuMon *cpumon)
{
    cpumon->temp = read_sysfs_uint64(cpumon->fd_temp) / 1000;
}

void get_freq_mhz(CpuMon *cpumon)
{
    uint64_t total = 0;
    int valid_readings = 0;

    for (int i = 0; i < cpumon->phy_count; i++)
    {
        if (cpumon->fd_freq[i] > 0)
        {
            uint64_t val = read_sysfs_uint64(cpumon->fd_freq[i]);
            if (val > 0)
            {
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

void get_load_avg(CpuMon *cpumon)
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

static inline uint8_t calc_core_usage
(
    uint64_t user, uint64_t nice, uint64_t system, uint64_t idle,
    uint64_t iowait, uint64_t irq, uint64_t softirq, uint64_t steal,
    uint64_t *prev_total, uint64_t *prev_idle
)
{
    uint64_t virt_idle = idle + iowait;
    uint64_t virt_work = user + nice + system + irq + softirq + steal;
    
    uint64_t total = virt_idle + virt_work;
    uint64_t diff_total = total - *prev_total;
    uint64_t diff_idle  = virt_idle - *prev_idle;

    *prev_total = total;
    *prev_idle  = virt_idle;

    if (diff_total == 0)
        return 0;

    return (uint8_t)(((diff_total - diff_idle) * 100) / diff_total);
}

static inline void push_to_history(CpuMon *cpumon, int cpu_id, uint8_t usage)
{
    int idx = (cpu_id * cpumon->graph_width) + cpumon->graph_head;
    cpumon->graph_hist[idx] = usage;
}

void parse_stats(CpuMon* cpumon)
{
    ssize_t bytes_read = pread(cpumon->fd_stat, cpumon->stat_buffer, cpumon->stat_buffer_size - 1, 0);
    if (bytes_read < 0)
        return;

    cpumon->stat_buffer[bytes_read] = '\0';
    char *p = cpumon->stat_buffer;
    skip_line(&p);

    for (size_t cpu_id = 0; cpu_id < cpumon->thread_count; cpu_id++)
    {
        if (*p != 'c')
            break;

        skip_to_digit(&p); str_to_uint64(&p);

        skip_to_digit(&p); uint64_t user = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t nice = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t system = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t idle = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t iowait = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t irq = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t softirq = str_to_uint64(&p);
        skip_to_digit(&p); uint64_t steal = str_to_uint64(&p);

        skip_line(&p);

        uint8_t usage = calc_core_usage
        (
            user, nice, system, idle, iowait, irq, softirq, steal,
            &cpumon->prev_total[cpu_id], 
            &cpumon->prev_idle[cpu_id]
        );

        cpumon->usage[cpu_id] = usage;
        
        push_to_history(cpumon, cpu_id, usage);
    }
    
    cpumon->graph_head = (cpumon->graph_head + 1) % cpumon->graph_width;
}
