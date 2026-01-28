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
    int8_t usage[CORES_N];
    char model[MODEL_LEN + 1];

    unsigned long long prev_total[CORES_N];
    unsigned long long prev_idle[CORES_N];
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


void parse_cpu_stats(char* raw_text, CPU_mon* cpumon)
{
    // A. "Fatiar" a primeira linha
    // O strtok usa o próprio raw_text. Ele não cria cópia.
    char* line = strtok(raw_text, "\n");

    while (line != NULL)
    {
        // B. Analisar a linha
        // Precisamos verificar se a linha começa com "cpu"
        // E pular a primeira linha que é o total (tem um espaço depois de cpu)
        if (strncmp(line, "cpu", 3) == 0 && line[3] != ' ')
        {
            int cpu_id;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

            // C. Extrair os números das colunas
            // cpu%d -> Lê o ID (0, 1, 2...)
            // %llu -> Lê "Long Long Unsigned" (números gigantes)
            int ret = sscanf(line, "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu",
                             &cpu_id, 
                             &user, &nice, &system, &idle, 
                             &iowait, &irq, &softirq, &steal);

            if (ret >= 9 && cpu_id < CORES_N) // Leu tudo certo?
            {
                // D. Matemática do Linux (Jiffies)
                unsigned long long active = user + nice + system + irq + softirq + steal;
                unsigned long long total_idle = idle + iowait;
                unsigned long long total = active + total_idle;

                // E. Calcular o Delta (Diferença entre Agora e Antes)
                unsigned long long diff_total = total - cpumon->prev_total[cpu_id];
                unsigned long long diff_idle  = total_idle - cpumon->prev_idle[cpu_id];

                // F. Calcular Porcentagem
                if (diff_total > 0) {
                    // (Total - Idle) = Tempo trabalhado
                    unsigned long long usage = (diff_total - diff_idle) * 100 / diff_total;
                    cpumon->usage[cpu_id] = (int8_t)usage;
                }

                // G. Salvar estado para a próxima volta
                cpumon->prev_total[cpu_id] = total;
                cpumon->prev_idle[cpu_id] = total_idle;
            }
        }

        // H. Pega a próxima "fatia" (próxima linha)
        line = strtok(NULL, "\n");
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

    char* text = r_first_n_rows("/proc/stat", 13);
    if (text != NULL)
    {
        parse_cpu_stats(text, cpumon);
        
        free(text);
    }
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
