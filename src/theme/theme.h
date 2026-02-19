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
    char blk[THM_STR_LEN];
    char red[THM_STR_LEN];
    char grn[THM_STR_LEN];
    char ylw[THM_STR_LEN];
    char blu[THM_STR_LEN];
    char mag[THM_STR_LEN];
    char cyn[THM_STR_LEN];
    char wht[THM_STR_LEN];

    // --- GRADIENTS ---
    char temp[16][THM_STR_LEN];
    char pct[8][THM_STR_LEN];

    // --- SEMANTIC COLORS (Module Specific) ---
    char cpu_bd[THM_STR_LEN];
    char mem_bd[THM_STR_LEN];
    char mem_free[THM_STR_LEN];

} Theme;

extern Theme theme;

void theme_init(int theme_idx);
void theme_toggle();

#endif
