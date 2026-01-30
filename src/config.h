#ifndef CONFIG_H
#define CONFIG_H

#define CLOCK  200000

#define HWMON_PATH  "/sys/class/hwmon/"
#define CPUINFO_PATH "/proc/cpuinfo"

#define MAX_LINE_LENGTH 256
#define MAX_PATH_SIZE 256
#define MAX_SENSORS   32
#define MAX_CORES   32

#define RESET   "\033[0m"
#define BOLD    "\033[1m"

// --- Cores de Texto (Foreground) ---
#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"

// --- Cores de Texto em Negrito/Brilhante (Bold) ---
#define B_BLACK   "\033[1;30m"
#define B_RED     "\033[1;31m"
#define B_GREEN   "\033[1;32m"
#define B_YELLOW  "\033[1;33m"
#define B_BLUE    "\033[1;34m"
#define B_MAGENTA "\033[1;35m"
#define B_CYAN    "\033[1;36m"
#define B_WHITE   "\033[1;37m"

// --- Cores de Fundo (Background) ---
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"
#define BG_NAVY    "\033[48;5;234m"

#endif
