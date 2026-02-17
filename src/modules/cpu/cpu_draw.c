#include <stdint.h>

#include "ui/ui.h"
#include "ui/term.h"
#include "util/util.h"

void draw_temp_ui(char **p, int x, int y)
{
    tui_at(p, x, y);
    APPEND_LIT(p, "  °C");
}

void draw_temp_data(char **p, int x, int y, int temp)
{
    tui_at(p, x, y);
    APPEND_LIT(p, TX_BOLD);

    int t_idx = (temp + 128) >> 4;
    if (t_idx < 0)
        t_idx = 0;
    if (t_idx > 15)
        t_idx = 15;

    append_str(p, gradient_temp[t_idx]);
    append_num(p, temp);

    APPEND_LIT(p, TX_FONT TX_NOBOLD);
}

void draw_freq_ui(char **p, int x, int y)
{
    tui_at(p, x, y);
    APPEND_LIT(p, "    GHz");
}

void draw_freq_data(char **p, int x, int y, int mhz)
{
    tui_at(p, x, y);
    APPEND_LIT(p, TX_BOLD);

    append_fixed_1d(p, mhz, 1000);

    APPEND_LIT(p, TX_NOBOLD);
}

void draw_label_ui(char **p, int id)
{
    APPEND_LIT(p, TX_BOLD "C");
    append_num(p, id);
    APPEND_LIT(p, TX_NOBOLD);

    *(*p)++ = ' ';
    if (id < 10)
        *(*p)++ = ' ';
}

void draw_usage_ui(char **p, int width)
{
    for (int i = 0; i < width; i++)
        *(*p)++ = ' ';

    APPEND_LIT(p, "    %");
}

void draw_usage_data(char **p, int usage)
{
    *(*p)++ = ' ';
    if (usage)
        append_str(p, gradient_perc[(usage >> 4) & 7]);
    else
        APPEND_LIT(p, TX_DIM1);

    if (usage < 100)
    {
        *(*p)++ = ' ';
        if (usage < 10)
            *(*p)++ = ' ';
    }
    append_num(p, usage);
}

void draw_load_avg_ui(char **p, int x, int y)
{
    tui_at(p, x, y);
    APPEND_LIT(p, "AVG ");
}

void draw_load_avg_data(char **p, int x, int y, uint32_t avg[3])
{
    tui_at(p, x, y);
    for (int k = 0; k < 3; k++)
    {
        uint32_t raw = avg[k];
        uint32_t l_idx = raw >> 16;
        if (l_idx > 7) l_idx = 7;

        append_str(p, gradient_perc[l_idx]);
        append_fixed_shift_2d(p, raw, 16);
        if (k < 2)
            *(*p)++ = ' ';
    }
    APPEND_LIT(p, TX_FONT);
}

void draw_uptime_ui(char **p, int x, int y)
{
    tui_at(p, x, y);
    APPEND_LIT(p, "up ");
}

void draw_uptime_data(char **p, int x, int y, int uptime)
{
    tui_at(p, x, y);

    uint32_t secs = uptime % 60;
    uint32_t rem  = uptime / 60;
    uint32_t mins = rem % 60;
    uint32_t hours = rem / 60;

    if (hours < 10)
        *(*p)++ = '0';
    append_num(p, hours);

    *(*p)++ = ':';
    append_two_digits(p, mins);
    
    *(*p)++ = ':';
    append_two_digits(p, secs);
}
