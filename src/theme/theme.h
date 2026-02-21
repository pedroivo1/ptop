#ifndef THEME_H
#define THEME_H

#define THM_STR_LEN 32

typedef struct {
    // --- TEXT ATTRIBUTES ---
    char tx_bold[THM_STR_LEN];
    char tx_nobold[THM_STR_LEN];
    char tx_reset[THM_STR_LEN];

    char bg[THM_STR_LEN];
    char fg[THM_STR_LEN];
    char dim[THM_STR_LEN];
    char dim_dark[THM_STR_LEN];

    // --- BASIC COLORS ---
    char black[THM_STR_LEN];
    char red[THM_STR_LEN];
    char green[THM_STR_LEN];
    char yellow[THM_STR_LEN];
    char blue[THM_STR_LEN];
    char magenta[THM_STR_LEN];
    char cyan[THM_STR_LEN];
    char white[THM_STR_LEN];

    // --- GRADIENTS ---
    char temp[16][THM_STR_LEN];
    char pct[8][THM_STR_LEN];

    // --- SEMANTIC COLORS (Module Specific) ---
    char cpu_bd[THM_STR_LEN];
    char mem_bd[THM_STR_LEN];
    char mem_free[THM_STR_LEN];

} Theme;

typedef enum {
    THEME_DARK,   // Vale 0
    THEME_LIGHT,  // Vale 1
} ThemeId;

extern Theme theme;

void set_theme(ThemeId theme_idx);
void toggle_theme();

#endif
