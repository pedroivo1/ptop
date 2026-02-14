#include <stdio.h>
#include "../src/theme/theme.h"

static const char* ctemp[16] =
{
    TG_TEMP_0, TG_TEMP_1, TG_TEMP_2, TG_TEMP_3, TG_TEMP_4, TG_TEMP_5, TG_TEMP_6, TG_TEMP_7,
    TG_TEMP_8, TG_TEMP_9, TG_TEMP_10, TG_TEMP_11, TG_TEMP_12, TG_TEMP_13, TG_TEMP_14, TG_TEMP_15
};

static const char* cperc[8] =
{
    TG_P0, TG_P1, TG_P2, TG_P3, TG_P4, TG_P5, TG_P6, TG_P7
};

static const char* blocks[6] = {" ", ".", ".", ":", ":", ":"};

void print_header(const char* title) {
    printf(TX_RESET "\n" TX_BOLD "%s" TX_NOBOLD "\n", title);
}

int main() {
    // --- TEMPERATURE ---
    print_header("TEMP (Start: -128 | Step: 16)");
    
    printf("Fluxo: ");
    for(int i = 0; i < 16; i++) { printf("%s███", ctemp[i]); }
    printf(TX_RESET "\n\n");

    printf(TX_RESET "| ID | Cor | ID Macro | INIT °C |  FINAL °C |\n");
    printf(TX_RESET "|----|-----|----------|---------|-----------|\n");
    
    for(int i = 0; i < 16; i++) {
        int temp_val = -128 + (i * 16);
        
        printf(TX_RESET "|%3d | %s███" TX_RESET " | TEMP_%-2d  | " TX_BOLD " %4d   " TX_NOBOLD "|" TX_BOLD " %s%4d °C   " TX_RESET "|\n", 
               i, ctemp[i], i, temp_val, ctemp[i], temp_val+15);
    }

    // --- PERCENTAGE ---
    print_header("USAGE (Start: 0 | Step: 16)");
    
    printf("Fluxo: ");
    for(int i = 0; i < 8; i++) { printf("%s███", cperc[i]); }
    printf(TX_RESET "\n\n");

    printf(TX_RESET "| ID | Cor | ID Macro |  INIT %%  |  FINAL %% |\n");
    printf(TX_RESET "|----|-----|----------|----------|----------|\n");

    for(int i = 0; i < 8; i++) {
        int perc_val = 0 + (i * 16);
        
        printf(TX_RESET "|%3d | %s███" TX_RESET " | PERC_%-2d  | " TX_BOLD "  %3d %%  " TX_NOBOLD "|" TX_BOLD " %s %3d %%   " TX_RESET "|\n", 
               i, cperc[i], i, perc_val, cperc[i], perc_val+15);
    }

    printf("\033[0m\n");
    return 0;
}
