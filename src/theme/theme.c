#include "theme/theme.h"
#include "util/util.h"
#include <string.h>

Theme theme;
static ThemeId current_theme;

typedef struct
{
   char const* bg;
   char const* fg;
   char const* dim;
   char const* dim_dark;

   char const* black;
   char const* red;
   char const* green;
   char const* yellow;
   char const* blue;
   char const* magenta;
   char const* cyan;
   char const* white;

   char const* temps[16];
   char const* pcts[8];

   char const* cpu_bd;
   char const* mem_bd;
   char const* mem_free;
} RawPalette;

static RawPalette const DARK = {
   .bg = "233",
   .fg = "254",
   .dim = "243",
   .dim_dark = "237",
   .black = "0",
   .red = "9",
   .green = "10",
   .yellow = "11",
   .blue = "12",
   .magenta = "13",
   .cyan = "14",
   .white = "15",
   .temps = {"21",
             "21",
             "27",
             "27",
             "33",
             "39",
             "45",
             "51",
             "87",
             "49",
             "46",
             "118",
             "226",
             "202",
             "196",
             "129"},
   .pcts = {"47", "82", "154", "190", "226", "208", "196", "129"},
   .cpu_bd = "65",
   .mem_bd = "101",
   .mem_free = "250"};

static RawPalette const LIGHT = {
   .bg = "255",
   .fg = "232",
   .dim = "244",
   .dim_dark = "250",
   .black = "252",
   .red = "160",
   .green = "34",
   .yellow = "136",
   .blue = "18",
   .magenta = "90",
   .cyan = "24",
   .white = "232",
   .temps = {"18",
             "24",
             "25",
             "31",
             "30",
             "36",
             "28",
             "34",
             "64",
             "100",
             "136",
             "166",
             "160",
             "124",
             "88",
             "53"},
   .pcts = {"28", "34", "70", "136", "166", "160", "124", "90"},
   .cpu_bd = "24",
   .mem_bd = "22",
   .mem_free = "242"};

// --- INTERN ---
static void set_fg(char dest[static THM_STR_LEN], char const code[static 1])
{
   char* p = dest;
   APPEND_LIT(&p, "\033[38;5;");
   append_str(&p, code);
   APPEND_LIT(&p, "m");
   *p = '\0';
}

static void set_bg(char dest[static THM_STR_LEN], char const code[static 1])
{
   char* p = dest;
   APPEND_LIT(&p, "\033[48;5;");
   append_str(&p, code);
   APPEND_LIT(&p, "m");
   *p = '\0';
}

static void apply_palette(RawPalette const p[static 1])
{
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

   set_fg(theme.black, p->black);
   set_fg(theme.red, p->red);
   set_fg(theme.green, p->green);
   set_fg(theme.yellow, p->yellow);
   set_fg(theme.blue, p->blue);
   set_fg(theme.magenta, p->magenta);
   set_fg(theme.cyan, p->cyan);
   set_fg(theme.white, p->white);

   for (size_t i = 0; i < 16; i++)
      set_fg(theme.temp[i], p->temps[i]);
   for (size_t i = 0; i < 8; i++)
      set_fg(theme.pct[i], p->pcts[i]);

   set_fg(theme.cpu_bd, p->cpu_bd);
   set_fg(theme.mem_bd, p->mem_bd);
   set_fg(theme.mem_free, p->mem_free);
}

// --- PUBLIC ---
void set_theme(ThemeId theme_idx)
{
   current_theme = theme_idx;
   switch (current_theme)
   {
      case THEME_DARK:
         apply_palette(&DARK);
         break;
      case THEME_LIGHT:
         apply_palette(&LIGHT);
         break;
      default:
         apply_palette(&DARK);
   }
}

void toggle_theme()
{
   if (current_theme == THEME_DARK)
      set_theme(THEME_LIGHT);
   else
      set_theme(THEME_DARK);
}
