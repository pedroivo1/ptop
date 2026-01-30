#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/sysinfo.h>

// Hardware Definitions
#define MODEL "i7-10750H"
#define CORES_N 12
#define PHY_CORES_N 6
#define HWMON_N 9

// Paths & Buffers
#define STAT_PATH "/proc/stat"
#define STAT_BUFF_LEN 1024 // 1664 para poder rodar por anos
#define OUT_BUFF_LEN 2048

// Settings
#define DELAY_mS 400

// Colors
#define BG_BLACK     "\033[48;5;232m"
#define BTOP_BLUE   "\033[38;5;75m"
#define BTOP_DIM    "\033[38;5;242m"
#define BTOP_GREEN  "\033[38;5;113m"
#define BTOP_ORANGE "\033[38;5;215m"
#define BTOP_RED    "\033[38;5;196m"
#define BTOP_WHITE  "\033[38;5;253m"
#define RESET       "\033[0m" BG_BLACK BTOP_WHITE


// ================================================================
// ============================ GLOBAL ============================
// ================================================================
volatile sig_atomic_t run = 1;
struct termios original_term;
typedef struct
{
    uint64_t prev_total[CORES_N];
    uint64_t prev_idle[CORES_N];
    uint32_t uptime;

    int fd_stat;
    int fd_temp[PHY_CORES_N];
    int fd_freq[CORES_N];
    uint32_t load_avg[3];

    uint16_t freq[CORES_N];
    int8_t temp[CORES_N];
    uint8_t usage[CORES_N];

} CPU_mon;

// ===============================================================
// ============================ UTILS ============================
// ===============================================================
int read_sysfs_int(int fd)
{
    char buffer[16];
    int val = 0;
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    lseek(fd, 0, SEEK_SET);

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

char* append_int(char *buffer, int val)
{
    if (val == 0)
    {
        *buffer++ = '0';
        return buffer;
    }

    char temp[16];
    int i = 0;
    while (val > 0)
    {
        temp[i++] = (val % 10) + '0';
        val /= 10;
    }
    while (i > 0)
    {
        *buffer++ = temp[--i];
    }

    return buffer;
}

char *append_str(char* buffer, const char* str)
{
    while (*str) *buffer++ = *str++;
    return buffer;
}

// =============================================================
// ============================ CPU ============================
// =============================================================
void init_cpumon(CPU_mon* cpumon, int hwmon_cpu_id)
{
    memset(cpumon, 0, sizeof(CPU_mon));
    cpumon->fd_stat = open(STAT_PATH, O_RDONLY);

    char path[64];
    for(int i = 0; i < PHY_CORES_N; i++)
    {
        snprintf(path, sizeof(path), "/sys/class/hwmon/hwmon%d/temp%d_input", hwmon_cpu_id, i+2);
        cpumon->fd_temp[i] = open(path, O_RDONLY);
    }
    for(int i = 0; i < CORES_N; i++)
    {
        snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", i);
        cpumon->fd_freq[i] = open(path, O_RDONLY);
    }
}

void cleanup_cpumon(CPU_mon* cpumon)
{
    close(cpumon->fd_stat);

    for(int i = 0; i < PHY_CORES_N; i++)
        close(cpumon->fd_temp[i]);

    for(int i = 0; i < CORES_N; i++)
        close(cpumon->fd_freq[i]);
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

void get_core_temp_c(CPU_mon* cpumon)
{
    for(int i = 0; i < PHY_CORES_N; i++)
    {
        cpumon->temp[i] = read_sysfs_int(cpumon->fd_temp[i]) / 1000;
        cpumon->temp[i+PHY_CORES_N] = cpumon->temp[i];
    }
}

void get_core_freq_mhz(CPU_mon* cpumon)
{
    for(int i = 0; i < CORES_N; i++)
        cpumon->freq[i] = read_sysfs_int(cpumon->fd_freq[i]) / 1000;
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
        else
        {
            break;
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
}

void get_system_load(CPU_mon* cpumon)
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

void cpu_update(CPU_mon* cpumon, int hwmon_cpu_id)
{
    get_core_freq_mhz(cpumon);
    get_core_temp_c(cpumon);
    parse_cpu_stats(cpumon);
    get_system_load(cpumon);
}

// =============================================================
// ============================ TUI ============================
// =============================================================
void setup_terminal() {
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON);

    printf("\033[?1049h\033[?25l%s\033[2J\033[H", BG_BLACK);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restore_terminal() {
    printf("\033[0m\033[?1049l\033[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void cpu_show(CPU_mon* cpumon)
{
    char buffer[OUT_BUFF_LEN];
    char *c = buffer;

    c = append_str(c, "\033[3;1H");
    c = append_str(c, RESET);

    for(int i = 0; i < CORES_N; i++)
    {
        // Label
        c = append_str(c, "C");
        c = append_int(c, i);

        // Frequência
        int mhz = cpumon->freq[i];
        c = append_str(c, "\033[7G");
        c = append_int(c, mhz / 1000);
        c = append_str(c, ".");
        c = append_int(c, (mhz % 1000) / 100);
        c = append_str(c, " GHz");

        // Temperatura
        int temp_val = cpumon->temp[i];
        char* color_temp = BTOP_BLUE;
        if (temp_val >= 60) color_temp = BTOP_ORANGE;
        if (temp_val >= 80) color_temp = BTOP_RED;

        c = append_str(c, "\033[18G");
        c = append_str(c, color_temp);
        c = append_int(c, temp_val);
        c = append_str(c, RESET);
        c = append_str(c, "°C");

        // Usage
        int usage = cpumon->usage[i];
        char* color_usage = BTOP_GREEN;
        if (usage >= 50) color_usage = BTOP_ORANGE;
        if (usage >= 85) color_usage = BTOP_RED;
        c = append_str(c, "\033[25G ");
        c = append_str(c, color_usage);
        if (usage < 10)
        {
            *c++ = ' ';
            *c++ = ' ';
        }
        else if (usage < 100)
        {
            *c++ = ' ';
        }
        c = append_int(c, usage);
        c = append_str(c, RESET);
        c = append_str(c, "%\n");
    }

    // Load AVG
    c = append_str(c, "\nLoad AVG:  ");
    for(int k = 0; k < 3; k++) {
        unsigned long raw = cpumon->load_avg[k];

        int whole = raw >> 16;
        int frac  = ((raw * 100) >> 16) % 100;

        char* l_color = (whole >= 2) ? BTOP_ORANGE : BTOP_WHITE;

        c = append_str(c, l_color);
        c = append_int(c, whole);
        c = append_str(c, ".");

        if (frac < 10) *c++ = '0';
        c = append_int(c, frac);

        if(k < 2) c = append_str(c, "   ");
    }
    c = append_str(c, RESET);

    // Uptime
    long up = cpumon->uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;

    c = append_str(c, "\nUptime: ");
    if (hours < 10) c = append_str(c, "0");
    c = append_int(c, hours);
    c = append_str(c, ":");
    if (mins < 10) c = append_str(c, "0");
    c = append_int(c, mins);
    c = append_str(c, ":");
    if (secs < 10) c = append_str(c, "0");
    c = append_int(c, secs);

    // print
    if (write(STDOUT_FILENO, buffer, c - buffer) == -1)
        perror("write failed");
}

// ==============================================================
// ============================ MAIN ============================
// ==============================================================
void cpumon_exit(int sig)
{
    run = 0;
}

int main()
{
    signal(SIGINT, cpumon_exit);

    int hwmon_cpu_id = get_coretemp_id();

    CPU_mon cpumon;
    init_cpumon(&cpumon, hwmon_cpu_id);
    int delay = DELAY_mS*1000;

    setup_terminal();
    printf("\033[H%s%s\t%dms\n", RESET, MODEL, DELAY_mS);
    while(run)
    {
        cpu_update(&cpumon, hwmon_cpu_id);
        cpu_show(&cpumon);
        usleep(delay);
    }

    cleanup_cpumon(&cpumon);
    restore_terminal();

    return 0;
}
