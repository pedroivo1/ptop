#include <stdio.h>
#include "theme/theme.h"

void print_header(char const title[static 1]) {
    printf("%s\033[K%s%s%s\033[K\n", theme.tx_reset, theme.tx_bold, title, theme.tx_nobold);
}

void show_theme(ThemeId theme_idx) {
    set_theme(theme_idx);

    // --- TEMPERATURE ---
    print_header("TEMP");
    
    printf("%s\033[KFluxo: ", theme.tx_reset);
    for(unsigned i = 0; i < 16; i++) { 
        printf("%s███", theme.temp[i]); 
    }
    
    printf("%s\033[K\n\033[K\n", theme.tx_reset);

    printf("%s\033[K| ID | Cor | ID Macro | INIT °C |  FINAL °C |\n", theme.tx_reset);
    printf("%s\033[K|----|-----|----------|---------|-----------|\n", theme.tx_reset);
    
    for(unsigned i = 0; i < 16; i++) {
        int temp_val = -128 + ((int)i * 16);
        
        printf("%s\033[K|%3u | %s███%s | TEMP_%-2u  | %s %4d   %s|%s %s%4d °C   %s|\n", 
               theme.tx_reset, 
               i, 
               theme.temp[i], 
               theme.tx_reset, 
               i, 
               theme.tx_bold, temp_val, theme.tx_nobold,
               theme.tx_bold, theme.temp[i], temp_val + 15, theme.tx_reset);
    }

    // --- PERCENTAGE ---
    print_header("USAGE");
    
    printf("%s\033[KFluxo: ", theme.tx_reset);
    for(unsigned i = 0; i < 8; i++) { 
        printf("%s███", theme.pct[i]); 
    }
    
    printf("%s\033[K\n\033[K\n", theme.tx_reset);

    printf("%s\033[K| ID | Cor | ID Macro |  INIT %%  |  FINAL %% |\n", theme.tx_reset);
    printf("%s\033[K|----|-----|----------|----------|----------|\n", theme.tx_reset);

    for(unsigned i = 0; i < 8; i++) {
        unsigned perc_val = i * 16;
        
        printf("%s\033[K|%3u | %s███%s | PERC_%-2u  | %s  %3u %%  %s|%s %s %3u %%   %s|\n", 
               theme.tx_reset, 
               i, 
               theme.pct[i], 
               theme.tx_reset, 
               i, 
               theme.tx_bold, perc_val, theme.tx_nobold,
               theme.tx_bold, theme.pct[i], perc_val + 15, theme.tx_reset);
    }

    printf("\033[0m\n");
}

int main(void) {
    show_theme(THEME_DARK);
    show_theme(THEME_LIGHT);
    return 0;
}
