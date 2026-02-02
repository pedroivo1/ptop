#include "display.h"
#include "utils.h"
#include "cfg.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

// --- COLORS ---
#define BG_BLACK    "\033[48;5;234m"
#define BTOP_BLUE   "\033[38;5;75m"
#define BTOP_DIM    "\033[38;5;242m"
#define BTOP_GREEN  "\033[38;5;113m"
#define BTOP_ORANGE "\033[38;5;215m"
#define BTOP_RED    "\033[38;5;196m"
#define BTOP_WHITE  "\033[38;5;253m"
#define PRESET      "\033[0m" BG_BLACK BTOP_WHITE
#define BOLD        "\033[1m"
#define NOBOLD      "\033[22m"

// --- TEMPERATURE ---
#define TEMP_0   "\033[38;5;21m"
#define TEMP_1   "\033[38;5;27m"
#define TEMP_2   "\033[38;5;33m"
#define TEMP_3   "\033[38;5;39m"
#define TEMP_4   "\033[38;5;45m"
#define TEMP_5   "\033[38;5;51m"
#define TEMP_6   "\033[38;5;50m"
#define TEMP_7   "\033[38;5;49m"
#define TEMP_8   "\033[38;5;47m"
#define TEMP_9   "\033[38;5;82m"
#define TEMP_10  "\033[38;5;154m"
#define TEMP_11  "\033[38;5;226m"
#define TEMP_12  "\033[38;5;214m"
#define TEMP_13  "\033[38;5;202m"
#define TEMP_14  "\033[38;5;196m"
#define TEMP_15  "\033[38;5;129m"

static const char* ctemp[16] =
{
    TEMP_0, TEMP_1, TEMP_2, TEMP_3, TEMP_4, TEMP_5, TEMP_6, TEMP_7,
    TEMP_8, TEMP_9, TEMP_10, TEMP_11, TEMP_12, TEMP_13, TEMP_14, TEMP_15
};

// --- PERCENTAGE ---
#define PERC_0   "\033[38;5;48m"
#define PERC_1   "\033[38;5;46m"
#define PERC_2   "\033[38;5;118m"
#define PERC_3   "\033[38;5;226m"
#define PERC_4   "\033[38;5;214m"
#define PERC_5   "\033[38;5;196m"
#define PERC_6   "\033[38;5;196m"
#define PERC_7   "\033[38;5;129m"

static const char* cperc[8] =
{
    PERC_0, PERC_1, PERC_2, PERC_3, PERC_4, PERC_5, PERC_6, PERC_7
};

// --- BOX ---
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_H  "─" 
#define BOX_V  "│"

#define UI_WIDTH  32
#define UI_HEIGHT 14
#define UI_TOP    1
#define UI_LEFT   1

static const char* blocks[6] = {" ", ".", ".", ":", ":", ":"};

static struct termios original_term;

static inline char *draw_box(char *p)
{
    p = append_str(p, "\033[38;5;240m");
    p = append_str(p, "\033[");
    p = append_int(p, UI_TOP);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT);
    p = append_str(p, "H");

    p = append_str(p, BOX_TL BOX_TR);
    p = append_str(p, BTOP_WHITE);
    p = append_str(p, MODEL);
    p = append_str(p, NOBOLD);
    p = append_str(p, "\033[38;5;240m");
    p = append_str(p, BOX_TL BOX_H BOX_TR);
    p = append_str(p, "    ");
    p = append_str(p, BOX_TL BOX_H BOX_TR);
    p = append_str(p, "       ");
    p = append_str(p,  BOX_TL);
    for (int i = 0; i < UI_WIDTH - MODEL_LEN - 21; i++) p = append_str(p, BOX_H);
    p = append_str(p,  BOX_TR);

    for (int i = 1; i < UI_HEIGHT - 1; i++)
    {
        p = append_str(p, "\033[");
        p = append_int(p, UI_TOP + i);
        p = append_str(p, ";");
        p = append_int(p, UI_LEFT);
        p = append_str(p, "H");
        p = append_str(p, BOX_V);
    }
    for (int i = 1; i < UI_HEIGHT - 1; i++)
    {
        p = append_str(p, "\033[");
        p = append_int(p, UI_TOP + i);
        p = append_str(p, ";");
        p = append_int(p, UI_LEFT + UI_WIDTH - 1);
        p = append_str(p, "H");
        p = append_str(p, BOX_V);
    }

    p = append_str(p, "\033[");
    p = append_int(p, UI_TOP + UI_HEIGHT - 1);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT);
    p = append_str(p, "H");
    p = append_str(p, BOX_BL);
    p = append_str(p, BOX_BR);
    for (int i = 0; i < 21; i++) p = append_str(p, " ");
    p = append_str(p, BOX_BL);
    for (int i = 0; i < UI_WIDTH - 32; i++) p = append_str(p, BOX_H);

    p = append_str(p, BOX_BR);
    p = append_str(p, BTOP_WHITE);
    p = append_int(p, DELAY_MS);
    p = append_str(p, "ms");
    p = append_str(p, "\033[38;5;240m");
    p = append_str(p, BOX_BL);
    p = append_str(p, BOX_BR);

    return p;
}

static inline char *draw_temperature(char *p, int temp, int row)
{
    p = APPEND_LIT(p, PRESET);
    p = append_str(p, "\033[");
    p = append_int(p, row);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT + 14);
    p = append_str(p, "H");
    int temp_val = temp;
    p = append_str(p, BOLD);
    p = append_str(p, ctemp[(temp_val + 128) >> 4]);
    p = append_int(p, temp_val);
    p = APPEND_LIT(p, BTOP_WHITE);
    p = APPEND_LIT(p, "°C");
    p = append_str(p, NOBOLD);

    return p;
}

static inline char *draw_frequency(char *p, int freq, int row)
{
    p = append_str(p, "\033[");
    p = append_int(p, row);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT + 21);
    p = append_str(p, "H");
    int mhz = freq;
    p = append_str(p, BOLD);
    p = append_int(p, mhz / 1000);
    p = APPEND_LIT(p, ".");
    p = append_int(p, (mhz % 1000) / 100);
    p = APPEND_LIT(p, " GHz");
    p = append_str(p, NOBOLD);

    return p;
}

static inline char *draw_uptime(char *p, int uptime, int row)
{
    p = append_str(p, "\033[");
    p = append_int(p, row);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT + 2);
    p = append_str(p, "H");
    long up = uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;
    p = append_str(p, "Up: ");
    if  (hours < 10) p = append_str(p, "0");
    p = append_int(p, hours);
    p = append_str(p, ":");
    if (mins < 10) p = append_str(p, "0");
    p = append_int(p, mins);
    p = append_str(p, ":");
    if (secs < 10) p = append_str(p, "0");
    p = append_int(p, secs);


    return p;
}

void setup_terminal()
{
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    char buf[1024];
    char *p = buf;
    p = append_str(p, "\033[?1049h\033[?25l");
    p = append_str(p, BG_BLACK);
    p = append_str(p, "\033[2J");
    p = append_str(p, "\033[H");
    p = append_str(p, PRESET);

    p = draw_box(p);

    if (write(STDOUT_FILENO, buf, p - buf) == -1)
        perror("write failed");
}

void restore_terminal()
{
    printf("\033[0m\033[?1049l\033[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void render_interface(CpuMonitor* cpumon)
{
    static char buf[OUT_BUFF_LEN] __attribute__((aligned(64)));
    char *p = buf;

    int row = UI_TOP;
    p = draw_temperature(p, cpumon->temp, row);
    row = UI_TOP;
    p = draw_frequency(p, cpumon->freq, row);

    for (int i = 0; i < CORES_N; i++)
    {
        row = UI_TOP + i + 1;
        p = append_str(p, "\033[");
        p = append_int(p, row);
        p = append_str(p, ";");
        p = append_int(p, UI_LEFT + 1);
        p = append_str(p, "H");
        p = APPEND_LIT(p, BOLD);
        p = APPEND_LIT(p, "C");
        p = append_int(p, i);
        p = APPEND_LIT(p, PRESET);
        
        p = append_str(p, "\033[");
        p = append_int(p, row);
        p = append_str(p, ";");
        p = append_int(p, UI_LEFT + 5);
        p = append_str(p, "H");

        for(int k = 0; k < GRAPH_WIDTH; k++)
        {
            int idx = (cpumon->graph_head + k) % GRAPH_WIDTH;
            int val = cpumon->graph_hist[i][idx];
            p = append_str(p, cperc[(val >> 4) & 7]);
            int block_idx = 0;
            if (val > 0)  block_idx = 1; 
            if (val > 20) block_idx = 2;
            if (val > 40) block_idx = 3;
            if (val > 60) block_idx = 4;
            if (val > 80) block_idx = 5;
            p = append_str(p, blocks[block_idx]);
        }

        p = append_str(p, "\033[");
        p = append_int(p, row);
        p = append_str(p, ";");
        p = append_int(p, UI_LEFT + UI_WIDTH - 5);
        p = append_str(p, "H");
        int usage = cpumon->usage[i];
        p = append_str(p, cperc[(usage >> 4) & 7]);
        if (usage < 10) {*p++ = ' '; *p++ = ' ';}
        else if (usage < 100) *p++ = ' ';
        p = append_int(p, usage);
        p = append_str(p, PRESET);
        p = APPEND_LIT(p, "%");
    }

    int load_row = UI_TOP + UI_HEIGHT - 1;
    p = append_str(p, "\033[");
    p = append_int(p, load_row);
    p = append_str(p, ";");
    p = append_int(p, UI_LEFT + 2);
    p = append_str(p, "H");
    p = append_str(p, "AVG: ");
    for (int k = 0; k < 3; k++)
    {
        unsigned long raw = cpumon->load_avg[k];
        int whole = raw >> 16;
        int frac  = ((raw * 100) >> 16) % 100;
        unsigned int l_idx = raw >> 17;
        if (l_idx > 7) l_idx = 7;
        
        p = append_str(p, cperc[l_idx]);
        p = append_int(p, whole);
        p = append_str(p, ".");
        if (frac < 10) *p++ = '0';
        p = append_int(p, frac);
        if (k < 2) p = append_str(p, "  ");
    }
    p = append_str(p, PRESET);

    row = UI_TOP + UI_HEIGHT;
    p = draw_uptime(p, cpumon->uptime, row);

    if (write(STDOUT_FILENO, buf, p - buf) == -1)
        perror("write failed");
}
