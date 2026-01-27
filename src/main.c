#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

#define CLOCK 200000

int main()
{
    signal(SIGINT, handle_exit);

    printf("\033[?1049h");
    printf("\033[?25l");
    
    int i = 0;
    while(1)
    {
        printf("\033[H");
        printf("Clocks: %d\n", i++);
        cpu_monitor();
        usleep(CLOCK);
    }
    return 0;
}
