#include <fcntl.h>
#include <sys/sysinfo.h>
#include <dirent.h>

#include "modules/cpu/cpu.h"
#include "common/utils.h"
#include "common/cfg.h"

int get_temp_id(void)
{
    DIR *dir = opendir("/sys/class/hwmon");
    if (!dir) return -1;

    struct dirent *entry;
    char path[256];
    char buf[64];
    int found_id = -1;
    char *p;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.' || strncmp(entry->d_name, "hwmon", 5) != 0)
            continue;

        p = path;
        APPEND_LIT(&p, "/sys/class/hwmon/");
        p = append_str(p, entry->d_name);
        APPEND_LIT(&p, "/name");
        
        *p = '\0'; 

        int fd = open(path, O_RDONLY);
        if (fd < 0) continue;

        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        close(fd);

        if (n > 0)
        {
            buf[n] = '\0';
            if (buf[n-1] == '\n') buf[n-1] = '\0';

            if (strcmp(buf, CORE_LABEL_NAME) == 0)
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

while (*p)
    {
        if (strncmp(p, "siblings", 8) == 0) {
            skip_to_digit(&p);
            siblings = (int)str_to_uint64(&p);
        }
        else if (strncmp(p, "cpu cores", 9) == 0) {
            skip_to_digit(&p);
            phy_cores = (int)str_to_uint64(&p);
        }
        else {
            skip_line(&p);
            continue;
        }

        if (siblings > 0 && phy_cores > 0) break;
        skip_line(&p);
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

void parse_stats(CpuMon* cpumon)
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
