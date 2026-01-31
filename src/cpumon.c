#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/sysinfo.h>

#define APPEND_LIT(buf, str) (memcpy(buf, str, sizeof(str)-1), buf + sizeof(str)-1)

// --- HARDWARE DEFINITIONS ---
#define MODEL "i7-10750H"
#define MODEL_LEN 9
#define CORES_N 12
#define PHY_CORES_N 6
#define HWMON_N 9

// --- PATHS & BUFFERS ---
#define STAT_PATH "/proc/stat"
#define STAT_BUFF_LEN 2048
#define OUT_BUFF_LEN 3072
#define CORE_LABEL_NAME "coretemp"

// --- SETTINGS ---
#define DELAY_mS 400

// --- COLORS ---
#define BG_BLACK    "\033[48;5;234m"
#define BTOP_BLUE   "\033[38;5;75m"
#define BTOP_DIM    "\033[38;5;242m"
#define BTOP_GREEN  "\033[38;5;113m"
#define BTOP_ORANGE "\033[38;5;215m"
#define BTOP_RED    "\033[38;5;196m"
#define BTOP_WHITE  "\033[38;5;253m"
#define PRESET       "\033[0m" BG_BLACK BTOP_WHITE
#define BOLD        "\033[1m"


// --- TEMPERATURE (-128 to +127) ---
// Logic: 16 steps using (val + 128) >> 4

// --- NEGATIVE RANGE ---
#define TEMP_0   "\033[38;5;21m"   // [-128 to -113]
#define TEMP_1   "\033[38;5;27m"   // [-112 to  -97]
#define TEMP_2   "\033[38;5;33m"   // [ -96 to  -81]
#define TEMP_3   "\033[38;5;39m"   // [ -80 to  -65]
#define TEMP_4   "\033[38;5;45m"   // [ -64 to  -49]
#define TEMP_5   "\033[38;5;51m"   // [ -48 to  -33]
#define TEMP_6   "\033[38;5;50m"   // [ -32 to  -17]
#define TEMP_7   "\033[38;5;49m"   // [ -16 to   -1]

// --- POSITIVE RANGE ---
#define TEMP_8   "\033[38;5;47m"   // [   0 to   15]
#define TEMP_9   "\033[38;5;82m"   // [  16 to   31]
#define TEMP_10  "\033[38;5;154m"  // [  32 to   47]
#define TEMP_11  "\033[38;5;226m"  // [  48 to   63]
#define TEMP_12  "\033[38;5;214m"  // [  64 to   79]
#define TEMP_13  "\033[38;5;202m"  // [  80 to   95]
#define TEMP_14  "\033[38;5;196m"  // [  96 to  111]
#define TEMP_15  "\033[38;5;129m"  // [ 112 to  127]

static const char* ctemp[16] = {
    TEMP_0, TEMP_1, TEMP_2, TEMP_3, TEMP_4, TEMP_5, TEMP_6, TEMP_7,
    TEMP_8, TEMP_9, TEMP_10, TEMP_11, TEMP_12, TEMP_13, TEMP_14, TEMP_15
};


// --- PERCENTAGE (0% to 100%) ---
#define PERC_0   "\033[38;5;48m"   // [  0 -  15]
#define PERC_1   "\033[38;5;46m"   // [ 16 -  31]
#define PERC_2   "\033[38;5;118m"  // [ 32 -  47]
#define PERC_3   "\033[38;5;226m"  // [ 48 -  63]
#define PERC_4   "\033[38;5;214m"  // [ 64 -  79]
#define PERC_5   "\033[38;5;196m"  // [ 80 -  95]
#define PERC_6   "\033[38;5;196m"  // [ 96 - 111]
#define PERC_7   "\033[38;5;129m"  // [112 - 127]

static const char* cperc[8] = {
    PERC_0, PERC_1, PERC_2,
    PERC_3, PERC_4, PERC_5,
    PERC_6, PERC_7
};

// --- BOX WALLS ---
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_H  "─" 
#define BOX_V  "│"

// --- BOX SIZES ---
#define UI_WIDTH  32
#define UI_HEIGHT 17
#define UI_TOP    1
#define UI_LEFT   1

#define GRAPH_WIDTH 12
static const char* blocks[6] = {"_", "▂", "▃", "▅", "▆", "▇"};

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
    int fd_temp;
    int fd_freq[PHY_CORES_N];
    uint32_t load_avg[3];

    uint16_t freq;
    uint8_t usage[CORES_N];
    int8_t temp;

} CPU_mon;


// ===============================================================
// ============================ UTILS ============================
// ===============================================================
static inline int read_sysfs_int(int fd)
{
    char buffer[16];
    int val = 0;
    ssize_t bytes_read = pread(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0)
    {
        char *p = buffer;
        while (*p >= '0' && *p <= '9')
        {
            val = (val * 10) + (*p++ - '0');
        }
    }
    return val;
}

static inline char* append_int(char *buffer, int val)
{
    if (val == 0) {
        *buffer++ = '0';
        return buffer;
    }

    char* start = buffer;
    while (val > 0) {
        *buffer++ = (val % 10) + '0';
        val /= 10;
    }

    char* end = buffer - 1;
    while (start < end) {
        char tmp_char = *start;
        *start++ = *end;
        *end-- = tmp_char;
    }

    return buffer;
}

static inline char* append_int_fast(char *buffer, int val)
{
    if (val == 0)
    {
        *buffer++ = '0';
        return buffer;
    }
    char temp[12];
    char *p_temp = temp;

    while (val > 0) {
        *p_temp++ = (val % 10) + '0';
        val /= 10;
    }

    while (p_temp > temp) {
        *buffer++ = *--p_temp;
    }
    return buffer;
}

static inline char *append_str(char* buffer, const char* str)
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
    char *p = path;
    p = append_str(p, "/sys/class/hwmon/hwmon");
    p = append_int_fast(p, hwmon_cpu_id);
    p = append_str(p, "/temp");
    p = append_int_fast(p, 1);
    p = append_str(p, "_input");
    *p = '\0';
    cpumon->fd_temp = open(path, O_RDONLY);

    for(int i = 0; i < PHY_CORES_N; i++)
    {
        p = path;
        p = append_str(p, "/sys/devices/system/cpu/cpu");
        p = append_int_fast(p, i);
        p = append_str(p, "/cpufreq/scaling_cur_freq");
        *p = '\0';
        cpumon->fd_freq[i] = open(path, O_RDONLY);
    }
}

void cleanup_cpumon(CPU_mon* cpumon)
{
    close(cpumon->fd_stat);
    close(cpumon->fd_temp);

    for(int i = 0; i < PHY_CORES_N; i++)
        close(cpumon->fd_freq[i]);
}

int get_coretemp_id()
{
    char path[32];
    char buffer[32];
    char *p;

    for(int i = 0; i < HWMON_N; i++)
    {
        p = path;
        p = append_str(p, "/sys/class/hwmon/hwmon");
        p = append_int_fast(p, i);
        p = append_str(p, "/name");
        *p = '\0';
        int fd = open(path, O_RDONLY);
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        if (bytes_read > 0) buffer[bytes_read] = '\0';

        char *p = buffer;
        int j = 0;
        while ( *p && j < 8)
        {
            if (*p++ != CORE_LABEL_NAME[j++]) break;
        }
        if (j == 8) return i;
    }

    return -1;
}

void get_core_temp_c(CPU_mon* cpumon)
{
    cpumon->temp = read_sysfs_int(cpumon->fd_temp) / 1000;
}

void get_core_freq_mhz(CPU_mon* cpumon)
{
    int total = 0;
    for(int i = 0; i < PHY_CORES_N; i++)
        total += read_sysfs_int(cpumon->fd_freq[i]);

    cpumon->freq = total / (PHY_CORES_N * 1000);
}

void parse_cpu_stats(CPU_mon* cpumon)
{
    static char buffer[STAT_BUFF_LEN]  __attribute__((aligned(64)));
    ssize_t bytes_read = pread(cpumon->fd_stat, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read < 0) return;

    char *p = buffer;
    while (*p && *p != '\n') p++;
    if (*p == '\n') p++;
    for (int cpu_id = 0; cpu_id < CORES_N; cpu_id++)
    {
        if (p[0] != 'c' || p[1] != 'p' || p[2] != 'u') break;

        p += 4;
        if (*p >= '0' && *p <= '9') p++;
        p++;

        uint64_t active = 0;
        uint64_t total_idle = 0;
        uint64_t val;

        // USER, NICE, SYSTEM
        for(int k=0; k<3; k++) {
            val = 0;
            while (*p >= '0') {
                val = (val * 10) + (*p++ - '0');
            }
            active += val;
            while (*p == ' ') p++;
        }

        // IDLE, IOWAIT
        for(int k=0; k<2; k++) {
            val = 0;
            while (*p >= '0') {
                val = (val * 10) + (*p++ - '0');
            }
            total_idle += val;
            while (*p == ' ') p++;
        }

        // IRQ, SOFTIRQ, STEAL
        for(int k=0; k<3; k++) {
            val = 0;
            while (*p >= '0') {
                val = (val * 10) + (*p++ - '0');
            }
            active += val;
            while (*p == ' ') p++; 
        }

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;

        uint64_t total = active + total_idle;
        uint64_t diff_total = total - cpumon->prev_total[cpu_id];
        uint64_t diff_idle  = total_idle - cpumon->prev_idle[cpu_id];

        if (diff_total > 0) {
            cpumon->usage[cpu_id] = (uint8_t)((diff_total - diff_idle) * 100 / diff_total);
        }

        cpumon->prev_total[cpu_id] = total;
        cpumon->prev_idle[cpu_id] = total_idle;
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
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    printf("\033[?1049h\033[?25l%s\033[2J", BG_BLACK);
    printf("\033[H%s", PRESET);
    fflush(stdout);

    char buffer[1024];
    char *p = buffer;

    p = append_str(p, "\033[38;5;245m");

    p = append_str(p, "\033[");
    p = append_int_fast(p, UI_TOP);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT);
    p = append_str(p, "H");

    p = append_str(p, BOX_TL);
    p = append_str(p, BOX_TR MODEL BOX_TL);
    for(int i = 0; i < UI_WIDTH - MODEL_LEN - 5; i++) p = append_str(p, BOX_H);
    p = append_str(p, BOX_H);
    p = append_str(p, BOX_TR);

    // --- LEFT SIDE ---
    for(int i = 1; i < UI_HEIGHT - 1; i++) {
        p = append_str(p, "\033[");
        p = append_int_fast(p, UI_TOP + i);
        p = append_str(p, ";");
        p = append_int_fast(p, UI_LEFT);
        p = append_str(p, "H");
        p = append_str(p, BOX_V);
    }

    // --- RIGHT SIDE ---
    for(int i = 1; i < UI_HEIGHT - 1; i++) {
        p = append_str(p, "\033[");
        p = append_int_fast(p, UI_TOP + i);
        p = append_str(p, ";");
        p = append_int_fast(p, UI_LEFT + UI_WIDTH - 1);
        p = append_str(p, "H");
        p = append_str(p, BOX_V);
    }

    // --- BOTTOM LINE ---
    p = append_str(p, "\033[");
    p = append_int_fast(p, UI_TOP + UI_HEIGHT - 1);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT);
    p = append_str(p, "H");
    p = append_str(p, BOX_BL);
    p = append_str(p, BOX_BR);
    for(int i = 0; i < 21; i++) p = append_str(p, BOX_H);
    p = append_str(p, BOX_BL);
    for(int i = 0; i < UI_WIDTH - 25; i++) p = append_str(p, BOX_H);
    p = append_str(p, BOX_BR);

    if (write(STDOUT_FILENO, buffer, p - buffer) == -1)
        perror("write failed");
}

void restore_terminal() {
    printf("\033[0m\033[?1049l\033[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void cpu_show(CPU_mon* cpumon)
{
    static char buffer[OUT_BUFF_LEN] __attribute__((aligned(64)));
    char *p = buffer;

    // --- MAIN LABEL ---
    p = APPEND_LIT(p, PRESET);
    p = append_str(p, "\033[");
    p = append_int_fast(p, UI_TOP + 1);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT + 2);
    p = append_str(p, "H");
    p = append_str(p, "CPU   ");

    // --- TEMPERATURE ---
    p = APPEND_LIT(p, PRESET);
    p = append_str(p, "\033[");
    p = append_int_fast(p, UI_TOP + 1);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT + 8);
    p = append_str(p, "H");
    int temp_val = cpumon->temp;
    p = append_str(p, ctemp[(temp_val + 128) >> 4]);
    p = append_int_fast(p, temp_val);
    p = APPEND_LIT(p, PRESET);
    p = APPEND_LIT(p, "°C");

    // --- FREQUENCY ---
    p = append_str(p, "\033[");
    p = append_int_fast(p, UI_TOP + 1);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT + 15);
    p = append_str(p, "H");
    int mhz = cpumon->freq;
    p = append_int_fast(p, mhz / 1000);
    p = APPEND_LIT(p, ".");
    p = append_int_fast(p, (mhz % 1000) / 100);
    p = APPEND_LIT(p, " GHz");

    for(int i = 0; i < CORES_N; i++)
    {
        int linha_atual = UI_TOP + i + 3;

        // --- LABEL ---
        p = append_str(p, "\033[");
        p = append_int_fast(p, linha_atual);
        p = append_str(p, ";");
        p = append_int_fast(p, UI_LEFT + 2);
        p = append_str(p, "H");
        p = APPEND_LIT(p, BOLD);
        p = APPEND_LIT(p, "C");
        p = append_int_fast(p, i);
        p = APPEND_LIT(p, PRESET);

        // --- USAGE ---
        p = append_str(p, "\033[");
        p = append_int_fast(p, linha_atual);
        p = append_str(p, ";");
        p = append_int_fast(p, UI_LEFT + UI_WIDTH - 5);
        p = append_str(p, "H");
        int usage = cpumon->usage[i];
        p = append_str(p, cperc[(usage >> 4) & 7]);
        if (usage < 10) {*p++ = ' '; *p++ = ' ';}
        else if (usage < 100) *p++ = ' ';

        p = append_int_fast(p, usage);
        p = append_str(p, PRESET);
        p = APPEND_LIT(p, "%");
    }

    // --- LOAD AVG ---
    int linha_load = UI_TOP + UI_HEIGHT - 1;
    p = append_str(p, "\033[");
    p = append_int_fast(p, linha_load);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT + 2);
    p = append_str(p, "H");
    p = append_str(p, "AVG: ");
    for(int k = 0; k < 3; k++) {
        unsigned long raw = cpumon->load_avg[k];

        int whole = raw >> 16;
        int frac  = ((raw * 100) >> 16) % 100;

        unsigned int l_idx = raw >> 17;
        if (l_idx > 7) l_idx = 7;
        
        p = append_str(p, cperc[l_idx]);
        p = append_int_fast(p, whole);
        p = append_str(p, ".");

        if (frac < 10) *p++ = '0';
        p = append_int_fast(p, frac);

        if(k < 2) p = append_str(p, "  ");
    }
    p = append_str(p, PRESET);

    // --- UPTIME ---
    int linha_up = UI_TOP + UI_HEIGHT + 1;
    p = append_str(p, "\033[");
    p = append_int_fast(p, linha_up);
    p = append_str(p, ";");
    p = append_int_fast(p, UI_LEFT);
    p = append_str(p, "H");
    
    long up = cpumon->uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;

    p = append_str(p, "Up: ");
    if (hours < 10) p = append_str(p, "0");
    p = append_int_fast(p, hours);
    p = append_str(p, ":");
    if (mins < 10) p = append_str(p, "0");
    p = append_int_fast(p, mins);
    p = append_str(p, ":");
    if (secs < 10) p = append_str(p, "0");
    p = append_int_fast(p, secs);

    // --- FINAL FLUSH ---
    if (write(STDOUT_FILENO, buffer, p - buffer) == -1)
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
