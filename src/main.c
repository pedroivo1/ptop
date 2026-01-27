#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"

#define CLOCK 200000

int main()
{
    signal(SIGINT, handle_exit);

    printf("\033[?1049h");
    printf("\033[?25l");

    const char *hwmon = "/sys/class/hwmon/";
    int hwmon_size = count_subdirs(hwmon);
    char *hwmon_names[hwmon_size];
    for(int i=0; i<hwmon_size; i++) hwmon_names[i] = NULL;
    while(ls_subdirs(hwmon, hwmon_names, hwmon_size) != hwmon_size);
    for(int i = 0; i < hwmon_size; i++)
    {
        printf("%s%s\n", hwmon, hwmon_names[i]);
    }

    sleep(1);
    int i = 0;
    while(1)
    {
        printf("\033[H");
        printf("Clocks: %d\n", i++);
        cpu_monitor();
        usleep(CLOCK);
    }

    for(int i = 0; i < hwmon_size; i++)
    {
        free(hwmon_names[i]);
    }

    return 0;
}
