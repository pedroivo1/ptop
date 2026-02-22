#include "util/rect.h"
#include <stddef.h>

// clang-format off
// --- BRAILE DOTS ---
#define DOTS_1   "\xE2\xA3\x80"  // ⣀
#define DOTS_2   "\xE2\xA3\xA0"  // ⣠
#define DOTS_3   "\xE2\xA3\xA4"  // ⣤
#define DOTS_4   "\xE2\xA3\xA6"  // ⣦
#define DOTS_5   "\xE2\xA3\xB6"  // ⣶
#define DOTS_6   "\xE2\xA3\xB7"  // ⣷
#define DOTS_7   "\xE2\xA3\xBF"  // ⣿
#define DOTS_8   "\xE2\xA3\xBF"  // ⣿

#define INV_DOTS_1   "\xE2\xA0\x89"  // ⠉
#define INV_DOTS_2   "\xE2\xA0\x99"  // ⠙
#define INV_DOTS_3   "\xE2\xA0\x9B"  // ⠛
#define INV_DOTS_4   "\xE2\xA0\x9F"  // ⠟
#define INV_DOTS_5   "\xE2\xA0\xBF"  // ⠿
#define INV_DOTS_6   "\xE2\xA1\xBF"  // ⡿
#define INV_DOTS_7   "\xE2\xA3\xBF"  // ⣿
#define INV_DOTS_8   "\xE2\xA3\xBF"  // ⣿

void tui_draw_box_graph(char** p,
                        uint8_t* data,
                        size_t capacity,
                        int head,
                        Rect r);
