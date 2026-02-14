#include "ui/ui.h"
#include "theme/theme.h"

char *draw_meter(char *p, int x, int y, int w, const char *label, const char *color_bar, int perc, uint64_t val_kb)
{
    int label_w = 12;
    int value_w = 11;
    int bar_w = w - label_w - value_w;
    if (bar_w < 5) bar_w = 5;

    p = tui_at(p, x, y);

    p = append_str(p, TX_FONT);
    p = append_str(p, label);
    APPEND_LIT(&p, ":");

    if (perc < 100) APPEND_LIT(&p, " ");
    if (perc < 10)  APPEND_LIT(&p, " ");

    p = append_str(p, color_bar);
    append_num(&p, perc);
    APPEND_LIT(&p, "%");
    APPEND_LIT(&p, " ");

    int fill = (bar_w * perc) / 100;
    for (int i = 0; i < fill; i++) *p++ = '|';
    p = append_str(p, TX_DIM0);
    for (int i = fill; i < bar_w; i++) *p++ = '|';

    p = append_str(p, TX_FONT);
    APPEND_LIT(&p, " ");
    p = append_fixed2(p, val_kb, 20, 100);
    if (val_kb < 10485760) APPEND_LIT(&p, " ");
    APPEND_LIT(&p, " GiB");

    return p;
}
