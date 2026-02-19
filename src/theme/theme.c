#include <string.h>
#include "util/util.h"
#include "theme/theme.h"
#include "theme.h"

Theme theme;
static int current_theme_id = 0;

typedef struct {
    char const* bg;
    char const* fg;
    char const* dim;
    char const* dim_dark;

    char const* blk;
    char const* red;
    char const* grn;
    char const* ylw;
    char const* blu;
    char const* mag;
    char const* cyn;
    char const* wht;

    char const* temps[16];
    char const* pcts[8];

    char const* cpu_bd;
    char const* mem_bd;
    char const* mem_free;
} RawPalette;

static RawPalette const DARK_PAL = {
    .bg = "233", .fg = "253", .dim = "243", .dim_dark = "237",
    .blk = "0",  .red = "1",   .grn = "2",   .ylw = "3",
    .blu = "4",  .mag = "5",   .cyn = "6",   .wht = "7",
    .temps = { "21", "21", "27", "27", "33", "39", "45", "51",
               "87", "49", "46", "118", "226", "202", "196", "129" },
    .pcts =  { "47", "82", "154", "190", "226", "208", "196", "129" },
    .cpu_bd = "65", .mem_bd = "101", .mem_free = "250"
};

static RawPalette const LIGHT_PAL = {
    .bg = "255", .fg = "234", .dim = "244", .dim_dark = "250",
    .blk = "252", .red = "124", .grn = "28",  .ylw = "136",
    .blu = "18",  .mag = "90",  .cyn = "24",  .wht = "232",
    .temps = { "18", "24", "25", "31", "30", "36", "28", "34",
               "64", "100", "136", "166", "160", "124", "88", "53" },
    .pcts =  { "28", "34", "70", "136", "166", "160", "124", "90" },
    .cpu_bd = "24", .mem_bd = "22", .mem_free = "242"
};

// --- INTERN ---
static void set_fg(char dest[static 1], char const code[static 1]) {
    char* p = dest;
    APPEND_LIT(&p, "\033[38;5;");
    append_str(&p, code);
    APPEND_LIT(&p, "m");
    *p = '\0';
}

static void set_bg(char dest[static 1], char const code[static 1]) {
    char* p = dest;
    APPEND_LIT(&p, "\033[48;5;");
    append_str(&p, code);
    APPEND_LIT(&p, "m");
    *p = '\0';
}

static void apply_palette(RawPalette const p[static 1]) {
    strcpy(theme.tx_bold, "\033[1m");
    strcpy(theme.tx_nobold, "\033[22m");

    set_bg(theme.bg, p->bg);
    set_fg(theme.fg, p->fg);
    set_fg(theme.dim, p->dim);
    set_fg(theme.dim_dark, p->dim_dark);

    char* ptr = theme.tx_reset;
    APPEND_LIT(&ptr, "\033[0m");
    append_str(&ptr, theme.bg);
    append_str(&ptr, theme.fg);
    *ptr = '\0';

    set_fg(theme.blk, p->blk); set_fg(theme.red, p->red);
    set_fg(theme.grn, p->grn); set_fg(theme.ylw, p->ylw);
    set_fg(theme.blu, p->blu); set_fg(theme.mag, p->mag);
    set_fg(theme.cyn, p->cyn); set_fg(theme.wht, p->wht);

    for (size_t i = 0; i < 16; i++) {
        set_fg(theme.temp[i], p->temps[i]);
    }
    for (size_t i = 0; i < 8; i++) {
        set_fg(theme.pct[i], p->pcts[i]);
    }

    set_fg(theme.cpu_bd, p->cpu_bd);
    set_fg(theme.mem_bd, p->mem_bd);
    set_fg(theme.mem_free, p->mem_free);
}

// --- PUBLIC ---
void theme_init(int theme_idx) {
    apply_palette(&DARK_PAL);
    current_theme_id = theme_idx;
}

void theme_toggle() {
    if (current_theme_id == 0) {
        apply_palette(&LIGHT_PAL);
        current_theme_id = 1;
    } else {
        apply_palette(&DARK_PAL);
        current_theme_id = 0;
    }
}
