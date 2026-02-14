#include "ui/ui.h"

char *draw_temp_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "  ");
    APPEND_LIT(&p, "°C");
    return p;
}

char *draw_temp_data(char *p, int x, int y, int temp)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, TX_BOLD);
    p = append_str(p, gradient_temp[(temp + 128) >> 4]);
    append_num(&p, temp);
    APPEND_LIT(&p, TX_FONT TX_NOBOLD);
    return p;
}

char *draw_freq_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "   ");
    APPEND_LIT(&p, " GHz");
    return p;
}

char *draw_freq_data(char *p, int x, int y, int mhz)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, TX_BOLD);
    p = append_fixed(p, mhz, 1000, 10);
    APPEND_LIT(&p, TX_NOBOLD);
    return p;
}

char *draw_label_ui(char *p, int id)
{
    APPEND_LIT(&p, TX_BOLD);
    APPEND_LIT(&p, "C");
    append_num(&p, id);
    APPEND_LIT(&p, TX_NOBOLD);
    *p++ = ' ';
    if (id < 10) *p++ = ' ';
    return p;
}

char *draw_usage_ui(char *p, int width)
{
    for (int i = 0; i < width; i++)
    {
        APPEND_LIT(&p, " ");
    }
    APPEND_LIT(&p, "    ");
    APPEND_LIT(&p, "%");
    return p;
}

char *draw_usage_data(char *p, int usage)
{
    APPEND_LIT(&p, " ");
    if (usage)
        p = append_str(p, gradient_perc[(usage >> 4) & 7]);
    else
        APPEND_LIT(&p, TX_DIM1);
    if (usage < 10) *p++ = ' ';
    if (usage < 100) *p++ = ' ';
    append_num(&p, usage);
    return p;
}

char *draw_load_avg_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "AVG ");

    return p;
}

char *draw_load_avg_data(char *p, int x, int y, uint32_t avg[3])
{
    p = tui_at(p, x, y);
    for (int k = 0; k < 3; k++)
    {
        uint32_t raw = avg[k];
        uint32_t l_idx = raw >> 16;
        if (l_idx > 7) l_idx = 7;

        p = append_str(p, gradient_perc[l_idx]);
        p = append_fixed(p, raw, 65536, 100);
        if (k < 2) APPEND_LIT(&p, " ");
    }
    APPEND_LIT(&p, TX_FONT);

    return p;
}

char *draw_uptime_ui(char *p, int x, int y)
{
    p = tui_at(p, x, y);
    APPEND_LIT(&p, "up ");
    return p;
}

char *draw_uptime_data(char *p, int x, int y, int uptime)
{
    p = tui_at(p, x, y);
    long up = uptime;
    int hours = up / 3600;
    int mins = (up % 3600) / 60;
    int secs = up % 60;
    if  (hours < 10) APPEND_LIT(&p, "0");
    append_num(&p, hours);
    APPEND_LIT(&p, ":");
    if (mins < 10) APPEND_LIT(&p, "0");
    append_num(&p, mins);
    APPEND_LIT(&p, ":");
    if (secs < 10) APPEND_LIT(&p, "0");
    append_num(&p, secs);
    return p;
}
