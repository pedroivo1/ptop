#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "utils.h"

#define CORES_N 12
#define PHY_CORES_N 6
#define MODEL_LEN 9
#define MODEL "i7-10750H"
#define HWMON_N 9

typedef struct
{
    int16_t freq[CORES_N];
    int8_t temp[CORES_N];
    char model[MODEL_LEN + 1];
} CPU_mon;

CPU_mon* new_cpumon()
{
    CPU_mon* cpumon = (CPU_mon*) malloc(sizeof(CPU_mon));

    if (cpumon != NULL)
    {
        for (int i = 0; i < CORES_N; i++)
        {
            cpumon->freq[i] = 0;
            cpumon->temp[i] = 0;
        }
        strcpy(cpumon->model, MODEL);
    }
    return cpumon;
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
    for(int i = 0; i < CORES_N; i++)
    {
        cpumon->freq[i] = get_core_freq_mhz(i);
    }
    for(int i = 0; i < PHY_CORES_N; i++)
    {
        cpumon->temp[i] = get_core_temp_c(i+2, hwmon_cpu_id);
        cpumon->temp[i+PHY_CORES_N] = cpumon->temp[i];
    }
}

void cpu_show(CPU_mon* cpumon)
{
    for(int i = 0; i < CORES_N; i++)
    {
        printf("C%d\t %d Mhz\t %d °C\n", i, cpumon->freq[i], cpumon->temp[i]);
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
