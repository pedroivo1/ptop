#ifndef THEME_H
#define THEME_H

// --- HELPER MACROS ---
#define ESC_FG(id)          "\033[38;5;" id "m"
#define ESC_BG(id)          "\033[48;5;" id "m"

// --- TC: BASE COLORS (0-15) ---
#define TC_BLK         ESC_FG("0")
#define TC_RED         ESC_FG("1")
#define TC_GRN         ESC_FG("2")
#define TC_YLW         ESC_FG("3")
#define TC_BLU         ESC_FG("4")
#define TC_MAG         ESC_FG("5")
#define TC_CYN         ESC_FG("6")
#define TC_WHT         ESC_FG("7")
#define TC_GRY         ESC_FG("8")
#define TC_BRED        ESC_FG("9")
#define TC_BGRN        ESC_FG("10")
#define TC_BYLW        ESC_FG("11")
#define TC_BBLU        ESC_FG("12")
#define TC_BMAG        ESC_FG("13")
#define TC_BCYN        ESC_FG("14")
#define TC_BWHT        ESC_FG("15")

// --- TX: UI STRUCTURE & EXTRAS ---
#define TX_BOLD             "\033[1m"
#define TX_NOBOLD           "\033[22m"
#define TX_BG          ESC_BG("233")
#define TX_FONT        ESC_FG("253")
#define TX_DIM1        ESC_FG("243")
#define TX_DIM0        ESC_FG("237")
#define TX_RESET            "\033[0m" TX_BG TX_FONT

// --- TG: TEMPERATURE GRADIENT (0-15) ---
#define TG_TEMP_0      ESC_FG("21")
#define TG_TEMP_1      ESC_FG("21")
#define TG_TEMP_2      ESC_FG("27")
#define TG_TEMP_3      ESC_FG("27")
#define TG_TEMP_4      ESC_FG("33")
#define TG_TEMP_5      ESC_FG("39")
#define TG_TEMP_6      ESC_FG("45")
#define TG_TEMP_7      ESC_FG("51")
#define TG_TEMP_8      ESC_FG("87")
#define TG_TEMP_9      ESC_FG("49")
#define TG_TEMP_10     ESC_FG("46")
#define TG_TEMP_11     ESC_FG("118")
#define TG_TEMP_12     ESC_FG("226")
#define TG_TEMP_13     ESC_FG("202")
#define TG_TEMP_14     ESC_FG("196")
#define TG_TEMP_15     ESC_FG("129")

// --- TG: PERCENTAGE GRADIENT (0-7) ---
#define TG_P0          ESC_FG("47")
#define TG_P1          ESC_FG("82")
#define TG_P2          ESC_FG("154")
#define TG_P3          ESC_FG("190")
#define TG_P4          ESC_FG("226")
#define TG_P5          ESC_FG("208")
#define TG_P6          ESC_FG("196")
#define TG_P7          ESC_FG("129")

// --- TC: SEMANTIC MODULE COLORS ---
#define TC_CPU_BD      ESC_FG("65")
#define TC_MEM_BD      ESC_FG("101")

// --- MEM MAPPING ---
#define TC_MEM_USED    TG_P6
#define TC_MEM_AVAIL   TG_P0
#define TC_MEM_CACHE   TG_P4
#define TC_MEM_FREE    ESC_FG("250")

#endif
