#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "ui/ui.h"

const char *gradient_temp[16] = {
    TG_TEMP_0, TG_TEMP_1, TG_TEMP_2, TG_TEMP_3, TG_TEMP_4, TG_TEMP_5, TG_TEMP_6, TG_TEMP_7,
    TG_TEMP_8, TG_TEMP_9, TG_TEMP_10, TG_TEMP_11, TG_TEMP_12, TG_TEMP_13, TG_TEMP_14, TG_TEMP_15
};

const char* dots_braille[8] = {
    DOTS_1, DOTS_2, DOTS_3, DOTS_4, DOTS_5, DOTS_6, DOTS_7, DOTS_8
};

const char* gradient_perc[8] = {
    TG_P0, TG_P1, TG_P2, TG_P3, TG_P4, TG_P5, TG_P6, TG_P7
};

static struct termios original_term;

int term_w = 0;
int term_h = 0;
static volatile sig_atomic_t win_resized = 1;

void tui_setup(char *bg_color, char *font_color)
{
    struct termios new_term;
    tcgetattr(STDIN_FILENO, &original_term);
    new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    signal(SIGWINCH, handle_winch);

    char buf[512];
    char *p = buf;
    // Enable Alternate Buffer
    p = append_str(p, "\033[?1049h");
    // Hide cursor
    p = append_str(p, "\033[?25l");
    // Enable Mouse Click Tracking
    p = append_str(p, "\033[?1000h");
    // Enable SGR Mouse Mode
    p = append_str(p, "\033[?1006h");
    // Clear entire screen
    p = append_str(p, "\033[2J");
    // Move cursor to Home position
    p = append_str(p, "\033[H");
    p = append_str(p, bg_color);
    p = append_str(p, font_color);

    if (write(STDOUT_FILENO, buf, p - buf) == -1)
        perror("write failed");
}

void tui_restore()
{
    char buf[512];
    char *p = buf;

    // Reset all attributes to default
    p = append_str(p, "\033[0m");
    // Disable Alternate Buffer
    p = append_str(p, "\033[?1049l");
    // Show cursor again
    p = append_str(p, "\033[?25h");
    // Disable Mouse Tracking
    p = append_str(p, "\033[?1000l");
    // Disable SGR Mouse Mode
    p = append_str(p, "\033[?1006l");

    if (write(STDOUT_FILENO, buf, p - buf) == -1)
        perror("write failed");

    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

char *tui_draw_box(char *p, int x, int y, int w, int h, char *color)
{
    p = append_str(p, color);

    p = tui_at(p, x, y);
    APPEND_LIT(&p, BOX_TL);
    for (int i = 0; i < w - 2; i++)
        APPEND_LIT(&p, BOX_H);
    APPEND_LIT(&p, BOX_TR);

    for (int i = 0; i < h - 2; i++)
    {
        p = tui_at(p, x+w-1, y+1+i);
        APPEND_LIT(&p, BOX_V);
    }

    for (int i = 0; i < h - 2; i++)
    {
        p = tui_at(p, x, y+1+i);
        APPEND_LIT(&p, BOX_V);
    }

    p = tui_at(p, x, y+h-1);
    APPEND_LIT(&p, BOX_BL);
    for (int i = 0; i < w - 2; i++)
        APPEND_LIT(&p, BOX_H);
    APPEND_LIT(&p, BOX_BR);

    return p;
}

char *tui_draw_up_space(char *p, int x, int y, int len)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, BOX_TR);
    for (int i = 0; i < len; i++)
        APPEND_LIT(&p, " ");
    APPEND_LIT(&p, BOX_TL);

    return p;
}

char *tui_draw_bottom_space(char *p, int x, int y, int len)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, BOX_BR);
    for (int i = 0; i < len; i++)
        APPEND_LIT(&p, " ");
    APPEND_LIT(&p, BOX_BL);

    return p;
}

char *tui_draw_graph(char *p, uint8_t *data, int len, int head)
{
    for (int i = 0; i < len; i++)
    {
        int idx = (head + i) % len;
        int val = data[idx];
        int dot_idx = (val >> 4) & 7;

        if (val > 0) {
            p = append_str(p, gradient_perc[dot_idx]);
        } else {
            APPEND_LIT(&p, TX_DIM0);
        }

        p = append_str(p, dots_braille[dot_idx]);
    }

    return p;
}

void tui_update_size()
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        term_w = 80;
        term_h = 24;
    } else {
        term_w = ws.ws_col;
        term_h = ws.ws_row;
    }
}

void handle_winch(int sig)
{
    (void)sig;
    win_resized = 1;
}

char *tui_begin_frame(char *p, int *resized)
{
    if (win_resized)
    {
        tui_update_size();
        APPEND_LIT(&p, "\033[2J");
        win_resized = 0;
        if (resized) *resized = 1;
    }
    else
    {
        if (resized) *resized = 0;
    }
    return p;
}
