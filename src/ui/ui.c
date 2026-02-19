#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "ui/ui.h"
#include "ui/term.h"
#include "util/rect.h"

char const* dots_braille[8] = {
    DOTS_1, DOTS_2, DOTS_3, DOTS_4, DOTS_5, DOTS_6, DOTS_7, DOTS_8
};

char const* dots_braille_inv[8] = {
    INV_DOTS_1, INV_DOTS_2, INV_DOTS_3, INV_DOTS_4,
    INV_DOTS_5, INV_DOTS_6, INV_DOTS_7, INV_DOTS_8
};

char const* gradient_temp[16] = {
    theme.temp[0], theme.temp[1], theme.temp[2], theme.temp[3],
    theme.temp[4], theme.temp[5], theme.temp[6], theme.temp[7],
    theme.temp[8], theme.temp[9], theme.temp[10], theme.temp[11],
    theme.temp[12], theme.temp[13], theme.temp[14], theme.temp[15]
};

char const* gradient_perc[8] = {
    theme.pct[0], theme.pct[1], theme.pct[2], theme.pct[3],
    theme.pct[4], theme.pct[5], theme.pct[6], theme.pct[7]
};

static struct termios original_term;
size_t term_w = 0;
size_t term_h = 0;
static volatile sig_atomic_t win_resized = 1;

void tui_handle_winch(int sig) {
    (void)sig;
    win_resized = 1;
}

void tui_update_size() {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        term_w = 80;
        term_h = 24;
    } else {
        term_w = ws.ws_col;
        term_h = ws.ws_row;
    }
}

void tui_begin_frame(bool resized[static true]) {
    if (win_resized) {
        tui_update_size();
        win_resized = 0;
        *resized = 1;
    }
}

void tui_setup(char *bg_color, char *font_color)
{
    if (tcgetattr(STDIN_FILENO, &original_term) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    struct termios new_term = original_term;
    new_term.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) == -1) {
        perror("tcsetattr");
        exit(1);
    }

    signal(SIGWINCH, tui_handle_winch);

    char buf[1024];
    char *p = buf;
    APPEND_LIT(&p, TUI_INIT_SEQ);
    append_str(&p, bg_color);
    append_str(&p, font_color);

    if (write(STDOUT_FILENO, buf, p - buf) == -1) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
        perror("write failed");
        exit(1);
    }
}

void tui_restore() {
    char buf[512];
    char *p = buf;

    APPEND_LIT(&p, TUI_EXIT_SEQ);
    if (write(STDOUT_FILENO, buf, p - buf) < 0){

    }
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);
}

void tui_draw_box(char **p, int x, int y, size_t w, size_t h, char *color) {
    append_str(p, color);

    tui_at(p, x, y);
    APPEND_LIT(p, BOX_TL);
    for (size_t i = 0; i < w - 2; i++) {
        APPEND_LIT(p, BOX_H);
    }
    APPEND_LIT(p, BOX_TR);

    for (size_t i = 0; i < h - 2; i++) {
        tui_at(p, x+w-1, y+1+i);
        APPEND_LIT(p, BOX_V);

        tui_at(p, x, y+1+i);
        APPEND_LIT(p, BOX_V);
    }

    tui_at(p, x, y+h-1);
    APPEND_LIT(p, BOX_BL);
    for (size_t i = 0; i < w - 2; i++) {
        APPEND_LIT(p, BOX_H);
    }
    APPEND_LIT(p, BOX_BR);
}

void tui_draw_up_space(char **p, int x, int y, size_t len) {
    tui_at(p, x, y);
    APPEND_LIT(p, BOX_TR);
    for (size_t i = 0; i < len; i++) {
        *(*p)++ = ' ';
    }
    APPEND_LIT(p, BOX_TL);
}

void tui_draw_bottom_space(char **p, int x, int y, size_t len) {
    tui_at(p, x, y);
    APPEND_LIT(p, BOX_BR);
    for (size_t i = 0; i < len; i++) {
        *(*p)++ = ' ';
    }
    APPEND_LIT(p, BOX_BL);
}

void tui_draw_graph(char **p, uint8_t *data, size_t len, size_t capacity, int head) {
    int last_dot_idx = -2;
    for (size_t i = 0; i < len; i++) {
        int idx = (head + capacity - len + i) % capacity;
        
        uint8_t val = data[idx];

        int dot_idx = (val >> 4) & 7;
        if (val == 0) {
            if (last_dot_idx != -1) {
                append_str(p, theme.dim_dark);
                last_dot_idx = -1;
            }
        } else {
            if (dot_idx != last_dot_idx) {
                append_str(p, gradient_perc[dot_idx]);
                last_dot_idx = dot_idx;
            }
        }
        append_str(p, dots_braille[dot_idx]);
    }
}

void tui_draw_graph_mirrored(char **p, uint8_t *data, size_t capacity, int head, Rect r) {
    size_t center_y = r.h / 2;
    int is_even_h = (r.h % 2 == 0);
    
    int max_dist = (r.h - 1) / 2;
    if (!is_even_h) {
        max_dist = (r.h / 2) - 1;
    }
    
    if (max_dist < 1) {
        max_dist = 1;
    }

    int total_subpixels = (max_dist + 1) * 8;

    for (size_t y = 0; y < r.h; y++) {
        tui_at(p, r.x, r.y + y);

        int dist;
        const char **current_dots;

        if (is_even_h) {
            if (y < center_y) {
                dist = (center_y - 1) - y;
                current_dots = dots_braille;
            } else {
                dist = y - center_y;
                current_dots = dots_braille_inv; 
            }
        } else {
            if (y <= center_y) {
                dist = center_y - y;
                current_dots = dots_braille;
            } else {
                dist = y - center_y - 1; 
                current_dots = dots_braille_inv;
            }
        }

        int color_idx = (dist << 3) / max_dist;
        
        if (color_idx > 6) {
            color_idx = 6; 
        }

        int row_start_pixel = dist * 8;
        int row_end_pixel   = (dist + 1) * 8;

        for (size_t x = 0; x < r.w; x++) {
            size_t idx = (head + capacity - r.w + x) % capacity;
            uint8_t val = data[idx];
            int needed_subpixels = (val * total_subpixels) / 100;

            if (needed_subpixels >= row_end_pixel) {
                append_str(p, gradient_perc[color_idx]);
                append_str(p, current_dots[7]);
            } else if (needed_subpixels > row_start_pixel) {
                int remainder = needed_subpixels - row_start_pixel;
                int char_idx = remainder - 1;
                
                if (char_idx < 0) {
                    char_idx = 0;
                }
                if (char_idx > 7) {
                    char_idx = 7;
                }

                append_str(p, gradient_perc[color_idx]);
                append_str(p, current_dots[char_idx]); 

            } else {
                if (dist == 0) {
                    append_str(p, gradient_perc[0]); 
                    append_str(p, current_dots[0]);  
                } else {
                    append_str(p, " ");
                }
            }
        }
    }
}
