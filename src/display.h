#ifndef DISPLAY_H
#define DISPLAY_H

#include "monitor.h"

// --- COLORS ---
#define BG_BLACK    "\033[48;5;234m"
#define GRAY        "\033[38;5;245m"
#define WHITE       "\033[38;5;253m"
#define PRESET      "\033[0m" BG_BLACK WHITE
#define BOLD        "\033[1m"
#define NOBOLD      "\033[22m"

// --- TEMPERATURE ---
#define TEMP_0   "\033[38;5;21m"
#define TEMP_1   "\033[38;5;21m"
#define TEMP_2   "\033[38;5;27m"
#define TEMP_3   "\033[38;5;27m"
#define TEMP_4   "\033[38;5;33m"
#define TEMP_5   "\033[38;5;39m"
#define TEMP_6   "\033[38;5;45m"
#define TEMP_7   "\033[38;5;51m"
#define TEMP_8   "\033[38;5;87m"
#define TEMP_9   "\033[38;5;49m"
#define TEMP_10  "\033[38;5;46m"
#define TEMP_11  "\033[38;5;118m"
#define TEMP_12  "\033[38;5;226m"
#define TEMP_13  "\033[38;5;202m"
#define TEMP_14  "\033[38;5;196m"
#define TEMP_15  "\033[38;5;129m"

// --- PERCENTAGE ---
#define PERC_0   "\033[38;5;47m"
#define PERC_1   "\033[38;5;82m"
#define PERC_2   "\033[38;5;154m"
#define PERC_3   "\033[38;5;190m"
#define PERC_4   "\033[38;5;226m"
#define PERC_5   "\033[38;5;208m"
#define PERC_6   "\033[38;5;196m"
#define PERC_7   "\033[38;5;129m"

// --- BOX ---
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_H  "─" 
#define BOX_V  "│"

#define UI_WIDTH  32
#define UI_HEIGHT 14
#define UI_TOP    1
#define UI_LEFT   1

void setup_terminal();
void restore_terminal();
void render_interface(CpuMonitor* cpumon);

#endif
