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
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define BOLD    "\033[1m"

#endif
