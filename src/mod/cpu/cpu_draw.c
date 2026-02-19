#include <stdint.h>
#include "ui/ui.h"
#include "ui/term.h"
#include "util/util.h"
#include "theme/theme.h"

void draw_temp_ui(char* p[static 1], int x, int y) {
    tui_at(p, x, y);
    append_str(p, theme.dim);
    APPEND_LIT(p, BOX_TR);
    append_str(p, theme.fg);
    APPEND_LIT(p, "  °C");
    append_str(p, theme.dim);
    APPEND_LIT(p, BOX_TL);
}

void draw_temp_data(char* p[static 1], int x, int y, int temp) {
    tui_at(p, x, y);
    append_str(p, theme.tx_bold);

    int t_idx = (temp + 128) >> 4;
    if (t_idx < 0) {
        t_idx = 0;
    }
    if (t_idx > 15) {
        t_idx = 15;
    }

    append_str(p, theme.temp[t_idx]);
    append_num(p, temp);

    append_str(p, theme.fg);
    append_str(p, theme.tx_nobold);
}

void draw_freq_ui(char* p[static 1], int x, int y) {
    tui_at(p, x, y);
    append_str(p, theme.dim);
    APPEND_LIT(p, BOX_TR);
    APPEND_LIT(p, theme.fg);
    APPEND_LIT(p, "    GHz");
    append_str(p, theme.dim);
    APPEND_LIT(p, BOX_TL);
}

void draw_freq_data(char* p[static 1], int x, int y, int mhz) {
    tui_at(p, x, y);
    append_str(p, theme.tx_bold);
    append_fixed_1d(p, mhz, 1000);
    append_str(p, theme.tx_nobold);
}

void draw_label_ui(char* p[static 1], int id) {
    append_str(p, theme.tx_bold);
    APPEND_LIT(p, "C");
    append_num(p, id);
    append_str(p, theme.tx_nobold);

    *(*p)++ = ' ';
    if (id < 10) {
        *(*p)++ = ' ';
    }
}

void draw_usage_ui(char* p[static 1], size_t width) {
    for (size_t i = 0; i < width; i++) {
        *(*p)++ = ' ';
    }
    APPEND_LIT(p, "    %");
}

void draw_usage_data(char* p[static 1], int usage) {
    *(*p)++ = ' ';
    if (usage) {
        append_str(p, theme.pct[(usage >> 4) & 7]);
    } else {
        append_str(p, theme.dim);
    }

    if (usage < 100) {
        *(*p)++ = ' ';
        if (usage < 10) {
            *(*p)++ = ' ';
        }
    }
    append_num(p, usage);
}

void draw_load_avg_ui(char* p[static 1], int x, int y) {
    tui_at(p, x, y);
    APPEND_LIT(p, "AVG ");
}

void draw_load_avg_data(char* p[static 1], int x, int y, uint32_t avg[3]) {
    tui_at(p, x, y);
    for (size_t k = 0; k < 3; k++) {
        uint32_t raw = avg[k];
        uint32_t l_idx = raw >> 16;
        if (l_idx > 7) {
            l_idx = 7;
        }

        append_str(p, theme.pct[l_idx]);
        append_fixed_shift_2d(p, raw, 16);
        if (k < 2) {
            *(*p)++ = ' ';
        }
    }
    append_str(p, theme.fg);
}

void draw_uptime_ui(char* p[static 1], int x, int y) {
    tui_at(p, x, y);
    APPEND_LIT(p, "up ");
}

void draw_uptime_data(char* p[static 1], int x, int y, int uptime) {
    tui_at(p, x, y);

    uint32_t secs = uptime % 60;
    uint32_t rem  = uptime / 60;
    uint32_t mins = rem % 60;
    uint32_t hours = rem / 60;

    if (hours < 10) {
        *(*p)++ = '0';
    }
    append_num(p, hours);

    *(*p)++ = ':';
    append_two_digits(p, mins);

    *(*p)++ = ':';
    append_two_digits(p, secs);
}
