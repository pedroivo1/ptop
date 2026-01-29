#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include "utils.h"

#define CORES_N 12
#define PHY_CORES_N 6
#define MODEL_LEN 9
#define MODEL "i7-10750H"
#define HWMON_N 9
#define STAT_BUFF_LEN 1664

typedef struct
{
    unsigned long long prev_total[CORES_N];
    unsigned long long prev_idle[CORES_N];

    int16_t freq[CORES_N];
    int8_t temp[CORES_N];
    int8_t usage[CORES_N];
    char model[MODEL_LEN + 1];

} CPU_mon;

CPU_mon* new_cpumon()
{
    CPU_mon* cpumon = (CPU_mon*) malloc(sizeof(CPU_mon));

    if (cpumon != NULL)
    {
        memset(cpumon, 0, sizeof(CPU_mon));
        memcpy(cpumon->model, MODEL, MODEL_LEN);
    }
    return cpumon;
}


void parse_cpu_stats(CPU_mon* cpumon)
{
    char buffer[STAT_BUFF_LEN];

    int fd = open("/proc/stat", O_RDONLY);
    ssize_t bytes_read = read(fd, buffer, STAT_BUFF_LEN - 1);
    close(fd);

    buffer[bytes_read] = '\0';

    char *p = buffer;
    while (*p)
    {
        if (p[0] == 'c' && p[1] ==  'p' && p[2] == 'u')
        {
            if (p[3] >= '0' && p[3] <= '9')
            {
                p += 3;

                int cpu_id = 0;
                while (*p >= '0' && *p <= '9')
                {
                    cpu_id = (cpu_id * 10) + (*p - '0');
                    p++;
                }

                if (cpu_id < CORES_N)
                {
                    uint64_t fields[8];

                    for (int i = 0; i < 8; i++)
                    {
                        while (*p == ' ') p++;

                        uint64_t val = 0;
                        while (*p >= '0' && *p <= '9')
                        {
                            val = (val * 10) + (*p - '0');
                            p++;
                        }
                        fields[i] = val;
                    }

                    // Matemática do uso (User + Nice + System + Irq + SoftIrq + Steal)
                    uint64_t active = fields[0] + fields[1] + fields[2] + fields[5] + fields[6] + fields[7];
                    uint64_t total_idle = fields[3] + fields[4]; // Idle + IOwait
                    uint64_t total = active + total_idle;

                    uint64_t diff_total = total - cpumon->prev_total[cpu_id];
                    uint64_t diff_idle  = total_idle - cpumon->prev_idle[cpu_id];

                    if (diff_total > 0) {
                        // Calcula % e guarda no int8_t
                        cpumon->usage[cpu_id] = (int8_t)((diff_total - diff_idle) * 100 / diff_total);
                    }

                    // Atualiza estado anterior
                    cpumon->prev_total[cpu_id] = total;
                    cpumon->prev_idle[cpu_id] = total_idle;
                }
            }
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
}

int get_coretemp_id()
{
    char* text;
    char path[256];

    for(int i = 0; i < HWMON_N; i++)
    {
        snprintf(path, sizeof(path), "/sys/class/hwmon/hwmon%d/name", i);
        text = r_file(path);
        if(text != NULL && strstr(text, "coretemp") != NULL)
        {
            free(text);
            return i;
        }

        if(text != NULL) free(text);
    }

    return -1;
}

long get_core_temp_c(int core_i, int hwmon_cpu_id)
{
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/hwmon/hwmon%d/temp%d_input", hwmon_cpu_id, core_i);

    long temp_mc = read_int_from_file(path);
    return temp_mc/1000;
}

long get_core_freq_mhz(int core_i)
{
    char path[256];
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core_i);

    long freq_khz = read_int_from_file(path);
    return freq_khz / 1000;
}

void cpu_update(CPU_mon* cpumon, int hwmon_cpu_id)
{
    // MHz
    for(int i = 0; i < CORES_N; i++)
    {
        cpumon->freq[i] = get_core_freq_mhz(i);
    }

    // °C
    for(int i = 0; i < PHY_CORES_N; i++)
    {
        cpumon->temp[i] = get_core_temp_c(i+2, hwmon_cpu_id);
        cpumon->temp[i+PHY_CORES_N] = cpumon->temp[i];
    }

    // %
    parse_cpu_stats(cpumon);
        
}

void cpu_show(CPU_mon* cpumon)
{
    for(int i = 0; i < CORES_N; i++)
    {
        printf("C%d\t", i);
        printf("%.1f GHz    ", cpumon->freq[i] / 1000.0);
        printf("%d °C    ", cpumon->temp[i]);
        printf("%d%%\n", cpumon->usage[i]);
    }
}


int main()
{
    signal(SIGINT, handle_exit);
    CPU_mon* cpumon = new_cpumon();

    int hwmon_cpu_id = get_coretemp_id();
    printf("\033[?1049h");
    while(1)
    {

        printf("\033[H");
        cpu_update(cpumon, hwmon_cpu_id);
        cpu_show(cpumon);
        usleep(500000);

    }
    printf("\033[?1049l");


    return 0;
}
