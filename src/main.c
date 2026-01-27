#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"
#include "config.h"


int main()
{
    signal(SIGINT, handle_exit);

    printf("\033[?1049h");
    printf("\033[?25l");

    int hwmon_size = count_subdirs(HWMON_PATH);
    char *hwmon_names[hwmon_size];
    for(int i=0; i<hwmon_size; i++) hwmon_names[i] = NULL;
    while(ls_subdirs(HWMON_PATH, hwmon_names, hwmon_size) != hwmon_size);
    for(int i = 0; i < hwmon_size; i++)
    {
        char path[256];
        snprintf(path, sizeof(path), "%s%s/name", HWMON_PATH, hwmon_names[i]);
        char *sensor_name = r_file(path);
        if (sensor_name) {
            printf("Sensor %d: %s", i, sensor_name);
            
            free(sensor_name);
        }
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
