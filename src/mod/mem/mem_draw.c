#include <stdint.h>
#include "util/util.h"
#include "ui/ui.h"
#include "theme/theme.h"

void draw_meter(char **p, int x, int y, int w, const char *label, const char *color_bar, int perc, uint64_t val_kb)
{
    int label_w = 12;
    int value_w = 9;
    int bar_w = w - label_w - value_w;
    if (bar_w < 5)
        bar_w = 5;

    tui_at(p, x, y);

    append_str(p, theme.fg);
    append_str(p, label);
    *(*p)++ = ':';

    if (perc < 100)
    {
        *(*p)++ = ' ';
        if (perc < 10)
            *(*p)++ = ' ';
    }

    append_str(p, color_bar);
    append_num(p, perc);
    *(*p)++ = '%';

    int fill = (bar_w * perc) / 100;
    for (int i = 0; i < fill; i++)
        *(*p)++ = '|';
    append_str(p, theme.dim_dark);
    for (int i = fill; i < bar_w; i++)
        *(*p)++ = '|';

    append_str(p, theme.fg);
    *(*p)++ = ' ';
    append_fixed_shift_2d(p, val_kb, 20);
    if (val_kb < 10485760)
        *(*p)++ = ' ';
    APPEND_LIT(p, " GiB");
}
