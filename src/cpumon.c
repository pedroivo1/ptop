#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "utils.h"

#define CORES_N 12
#define MODEL_LEN 9
#define MODEL "i7-10750H"

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

long get_core_freq_mhz(int core_i)
{
    char path[256];
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core_i);

    FILE* fptr = fopen(path, "r");
    if(!fptr) return -1;

    long freq_khz = 0;
    if (fscanf(fptr, "%ld", &freq_khz) != 1) 
    {
        fclose(fptr);
        return -1;
    }
    if (freq_khz == -1) return -1;

    fclose(fptr);
    return freq_khz / 1000;
}

void cpu_update(CPU_mon* cpumon)
{
    for(int i = 0; i < CORES_N; i++)
    {
        cpumon->freq[i] = get_core_freq_mhz(i);
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
    CPU_mon* cpumon = new_cpumon();
    printf("\033[?1049h");
    while(1)
    {
        printf("\033[H");
        cpu_update(cpumon);
        cpu_show(cpumon);
        usleep(500000);

    }
    printf("\033[?1049l");

    return 0;
}
