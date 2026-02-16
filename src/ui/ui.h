#ifndef TUI_H
#define TUI_H

#include <stdint.h>
#include "common/utils.h"
#include "theme/theme.h"

// --- BRAILE DOTS ---
#define DOTS_1   "\xE2\xA3\x80"  // ⣀
#define DOTS_2   "\xE2\xA3\xA0"  // ⣠
#define DOTS_3   "\xE2\xA3\xA4"  // ⣤
#define DOTS_4   "\xE2\xA3\xA6"  // ⣦
#define DOTS_5   "\xE2\xA3\xB6"  // ⣶
#define DOTS_6   "\xE2\xA3\xB7"  // ⣷
#define DOTS_7   "\xE2\xA3\xBF"  // ⣿
#define DOTS_8   "\xE2\xA3\xBF"  // ⣿

// --- BOX ---
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_H  "─"
#define BOX_V  "│"

extern const char* gradient_temp[16];
extern const char* gradient_perc[8];
extern const char* dots_braille[8];

static inline void tui_at(char **p, int x, int y)
{
    APPEND_LIT(p, "\033[");
    append_num(p, y);
    *(*p)++ = ';';
    append_num(p, x);
    *(*p)++ = 'H';
}

void tui_setup(char *bg_color, char *font_color);
void tui_restore();

void tui_draw_box(char **p, int x, int y, int w, int h, char *color);
void tui_draw_up_space(char **p, int x, int y, int len);
void tui_draw_bottom_space(char **p, int x, int y, int len);
void tui_draw_graph(char **p, uint8_t *data, int len, int head);

extern int term_w;
extern int term_h;
void tui_handle_winch(int sig);
void tui_begin_frame(char **p, int *resized);
void tui_update_size();

#endif
