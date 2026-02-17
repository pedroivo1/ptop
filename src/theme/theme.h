#ifndef THEME_H
#define THEME_H

#include "palette/dark.h"

// --- HELPER MACROS ---
#define ESC_FG(id)          "\033[38;5;" id "m"
#define ESC_BG(id)          "\033[48;5;" id "m"

// --- TX: UI STRUCTURE ---
#define TX_BOLD             "\033[1m"
#define TX_NOBOLD           "\033[22m"
#define TX_BG          ESC_BG(PAL_BG)
#define TX_FONT        ESC_FG(PAL_FG)
#define TX_DIM1        ESC_FG(PAL_DIM)
#define TX_DIM0        ESC_FG(PAL_DIM_DARK)
#define TX_RESET            "\033[0m" TX_BG TX_FONT

// --- TC: BASE COLORS ---
#define TC_BLK         ESC_FG(PAL_BLK)
#define TC_RED         ESC_FG(PAL_RED)
#define TC_GRN         ESC_FG(PAL_GRN)
#define TC_YLW         ESC_FG(PAL_YLW)
#define TC_BLU         ESC_FG(PAL_BLU)
#define TC_MAG         ESC_FG(PAL_MAG)
#define TC_CYN         ESC_FG(PAL_CYN)
#define TC_WHT         ESC_FG(PAL_WHT)

// --- TG: TEMPERATURE GRADIENT (0-15) ---
#define TG_TEMP_0      ESC_FG(PAL_T0)
#define TG_TEMP_1      ESC_FG(PAL_T1)
#define TG_TEMP_2      ESC_FG(PAL_T2)
#define TG_TEMP_3      ESC_FG(PAL_T3)
#define TG_TEMP_4      ESC_FG(PAL_T4)
#define TG_TEMP_5      ESC_FG(PAL_T5)
#define TG_TEMP_6      ESC_FG(PAL_T6)
#define TG_TEMP_7      ESC_FG(PAL_T7)
#define TG_TEMP_8      ESC_FG(PAL_T8)
#define TG_TEMP_9      ESC_FG(PAL_T9)
#define TG_TEMP_10     ESC_FG(PAL_T10)
#define TG_TEMP_11     ESC_FG(PAL_T11)
#define TG_TEMP_12     ESC_FG(PAL_T12)
#define TG_TEMP_13     ESC_FG(PAL_T13)
#define TG_TEMP_14     ESC_FG(PAL_T14)
#define TG_TEMP_15     ESC_FG(PAL_T15)

// --- TG: PERCENTAGE GRADIENT (0-7) ---
#define TG_P0          ESC_FG(PAL_P0)
#define TG_P1          ESC_FG(PAL_P1)
#define TG_P2          ESC_FG(PAL_P2)
#define TG_P3          ESC_FG(PAL_P3)
#define TG_P4          ESC_FG(PAL_P4)
#define TG_P5          ESC_FG(PAL_P5)
#define TG_P6          ESC_FG(PAL_P6)
#define TG_P7          ESC_FG(PAL_P7)

// --- TC: SEMANTIC MODULE COLORS ---
#define TC_CPU_BD      ESC_FG(PAL_CPU_BORDER)
#define TC_MEM_BD      ESC_FG(PAL_MEM_BORDER)

// --- MEM MAPPING ---
#define TC_MEM_USED    TG_P6
#define TC_MEM_AVAIL   TG_P0
#define TC_MEM_CACHE   TG_P4
#define TC_MEM_FREE    ESC_FG(PAL_MEM_FREE)

#endif
