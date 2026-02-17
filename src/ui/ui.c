#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "ui/ui.h"
#include "ui/term.h"

const char *gradient_temp[16] =
{
    TG_TEMP_0, TG_TEMP_1, TG_TEMP_2, TG_TEMP_3, TG_TEMP_4,
    TG_TEMP_5, TG_TEMP_6, TG_TEMP_7, TG_TEMP_8, TG_TEMP_9,
    TG_TEMP_10, TG_TEMP_11, TG_TEMP_12, TG_TEMP_13,
    TG_TEMP_14, TG_TEMP_15
};

const char *dots_braille[8] =
{
    DOTS_1, DOTS_2, DOTS_3, DOTS_4, DOTS_5, DOTS_6, DOTS_7, DOTS_8
};

const char *gradient_perc[8] =
{
    TG_P0, TG_P1, TG_P2, TG_P3, TG_P4, TG_P5, TG_P6, TG_P7
};

static struct termios original_term;
int term_w = 0;
int term_h = 0;
static volatile sig_atomic_t win_resized = 1;

void tui_handle_winch(int sig)
{
    (void)sig;
    win_resized = 1;
}

void tui_update_size()
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
    {
        term_w = 80;
        term_h = 24;
    }
    else
    {
        term_w = ws.ws_col;
        term_h = ws.ws_row;
    }
}

void tui_begin_frame(char **p, int *resized)
{
    if (win_resized)
    {
        tui_update_size();
        APPEND_LIT(p, "\033[2J");
        win_resized = 0;
        if (resized)
            *resized = 1;
    }
    else
    {
        if (resized)
            *resized = 0;
    }
}

void tui_setup(char *bg_color, char *font_color)
{
    if (tcgetattr(STDIN_FILENO, &original_term) == -1)
    {
        perror("tcgetattr");
        exit(1);
    }

    struct termios new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1)
    {
        perror("tcsetattr");
        exit(1);
    }

    signal(SIGWINCH, tui_handle_winch);

    char buf[1024];
    char *p = buf;
    APPEND_LIT(&p, TUI_INIT_SEQ);
    append_str(&p, bg_color);
    append_str(&p, font_color);

    if (write(STDOUT_FILENO, buf, p - buf) == -1)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
        perror("write failed");
        exit(1);
    }
}

void tui_restore()
{
    char buf[512];
    char *p = buf;

    APPEND_LIT(&p, TUI_EXIT_SEQ);
    if (write(STDOUT_FILENO, buf, p - buf) < 0){}
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void tui_draw_box(char **p, int x, int y, int w, int h, char *color)
{
    append_str(p, color);

    tui_at(p, x, y);
    APPEND_LIT(p, BOX_TL);
    for (int i = 0; i < w - 2; i++)
        APPEND_LIT(p, BOX_H);
    APPEND_LIT(p, BOX_TR);

    for (int i = 0; i < h - 2; i++)
    {
        tui_at(p, x+w-1, y+1+i);
        APPEND_LIT(p, BOX_V);

        tui_at(p, x, y+1+i);
        APPEND_LIT(p, BOX_V);
    }

    tui_at(p, x, y+h-1);
    APPEND_LIT(p, BOX_BL);
    for (int i = 0; i < w - 2; i++)
        APPEND_LIT(p, BOX_H);
    APPEND_LIT(p, BOX_BR);
}

void tui_draw_up_space(char **p, int x, int y, int len)
{
    tui_at(p, x, y);
    APPEND_LIT(p, BOX_TR);
    for (int i = 0; i < len; i++)
        *(*p)++ = ' ';
    APPEND_LIT(p, BOX_TL);
}

void tui_draw_bottom_space(char **p, int x, int y, int len)
{
    tui_at(p, x, y);
    APPEND_LIT(p, BOX_BR);
    for (int i = 0; i < len; i++)
        *(*p)++ = ' ';
    APPEND_LIT(p, BOX_BL);
}

void tui_draw_graph(char **p, uint8_t *data, int len, int head)
{
    int last_dot_idx = -2;
    for (int i = 0; i < len; i++)
    {
        int idx = (head + i) % len;
        uint8_t val = data[idx];

        int dot_idx = (val >> 4) & 7;
        if (val == 0)
        {
            if (last_dot_idx != -1)
            {
                APPEND_LIT(p, TX_DIM0);
                last_dot_idx = -1;
            }
        }
        else
        {
            if (dot_idx != last_dot_idx)
            {
                append_str(p, gradient_perc[dot_idx]);
                last_dot_idx = dot_idx;
            }
        }
        append_str(p, dots_braille[dot_idx]);
    }
}
