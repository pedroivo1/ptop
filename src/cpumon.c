#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#define CORES_N 12
#define PHY_CORES_N 6
#define MODEL_LEN 9
#define MODEL "i7-10750H"
#define HWMON_N 9
#define STAT_BUFF_LEN 1664
#define STAT_PATH "/proc/stat"
#define DELAY_mS 400

volatile sig_atomic_t run = 1;

struct termios original_term;

typedef struct
{
    uint64_t prev_total[CORES_N];
    uint64_t prev_idle[CORES_N];

    int fd_stat;

    int16_t freq[CORES_N];
    int8_t temp[CORES_N];
    int8_t usage[CORES_N];

} CPU_mon;

CPU_mon* init_cpumon(CPU_mon* cpumon)
{
    memset(cpumon, 0, sizeof(CPU_mon));
    cpumon->fd_stat = open(STAT_PATH, O_RDONLY);

    return cpumon;
}


void parse_cpu_stats(CPU_mon* cpumon)
{
    char buffer[STAT_BUFF_LEN];

    ssize_t bytes_read = read(cpumon->fd_stat, buffer, STAT_BUFF_LEN - 1);
    lseek(cpumon->fd_stat, 0, SEEK_SET);

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
                    
                    uint64_t active = 0;
                    uint64_t total_idle = 0;
                    uint64_t val;

                    // /proc/stat: user, nice, system, idle, iowait, irq, softirq, steal
                    // Column 0: User (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    // Column 1: Nice (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    // Column 2: System (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    // Column 3: Idle (Idle)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    total_idle += val;

                    // Column 4: IOwait (Idle)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    total_idle += val;

                    // Column 5: Irq (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    // Column 6: SoftIrq (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    // Column 7: Steal (Active)
                    while (*p == ' ') p++;
                    val = 0;
                    while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
                    active += val;

                    uint64_t total = active + total_idle;
                    uint64_t diff_total = total - cpumon->prev_total[cpu_id];
                    uint64_t diff_idle  = total_idle - cpumon->prev_idle[cpu_id];

                    if (diff_total > 0)
                        cpumon->usage[cpu_id] = (int8_t)((diff_total - diff_idle) * 100 / diff_total);

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
    char path[32];
    char buffer[32];

    for(int i = 0; i < HWMON_N; i++)
    {
        snprintf(path, sizeof(path), "/sys/class/hwmon/hwmon%d/name", i);
        int fd = open(path, O_RDONLY);
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        if (bytes_read > 0) buffer[bytes_read] = '\0';
        if(strstr(buffer, "coretemp") != NULL)
        {
            return i;
        }
    }

    return -1;
}


long read_sysfs_int(const char* path)
{
    char buffer[16];
    int val = 0;
    int fd = open(path, O_RDONLY);
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        char *p = buffer;
        while (*p >= '0' && *p <= '9')
        {
            val = (val * 10) + (*p++ - '0');
        }
    }
    return val;
}

long get_core_temp_c(int core_i, int hwmon_cpu_id)
{
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/hwmon/hwmon%d/temp%d_input", hwmon_cpu_id, core_i);

    long temp_mc = read_sysfs_int(path);
    return temp_mc/1000;
}

long get_core_freq_mhz(int core_i)
{
    char path[256];
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", core_i);

    long freq_khz = read_sysfs_int(path);
    return freq_khz / 1000;
}

void cpu_update(CPU_mon* cpumon, int hwmon_cpu_id)
{
    // MHz
    for(int i = 0; i < CORES_N; i++)
        cpumon->freq[i] = get_core_freq_mhz(i);

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
        printf("C%d", i);
        printf("\033[7G%.1f GHz", cpumon->freq[i] / 1000.0);
        printf("\033[18G%d°C", cpumon->temp[i]);
        printf("\033[25G%3d%%\n", cpumon->usage[i]);
    }
}


void cpumon_exit(int sig)
{
    run = 0;
}


void setup_terminal() {
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON); 
    printf("\033[?25l");
    printf("\033[?1049h");
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
    printf("\033[?25h");
    printf("\033[?1049l");
}


int main()
{
    signal(SIGINT, cpumon_exit);

    CPU_mon cpumon;
    init_cpumon(&cpumon);
    int delay = DELAY_mS*1000;

    int hwmon_cpu_id = get_coretemp_id();
    setup_terminal();
    printf("\033[H");
    printf("%s\t%dms\n", MODEL, DELAY_mS);
    while(run)
    {
        printf("\033[3;1H");
        cpu_update(&cpumon, hwmon_cpu_id);
        cpu_show(&cpumon);
        usleep(delay);
    }

    close(cpumon.fd_stat);
    restore_terminal();

    return 0;
}
