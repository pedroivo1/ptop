#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "cfg.h"
#include "modules/cpu.h"
#include "tui.h"

volatile sig_atomic_t run = 1;

void handle_sigint(int sig)
{
    run = 0;
}

int main()
{
    signal(SIGINT, handle_sigint);

    CpuMonitor cpumon;
    init_cpumon(&cpumon);

    static char buf[OUT_BUFF_LEN] __attribute__((aligned(64)));

    int delay = DELAY_MS * 1000;
    int redraw = 1;
    tui_setup(BG_BLACK, WHITE);
    while (run)
    {
        char *p = buf;
        p = tui_begin_frame(p, &redraw);

        update_cpu_metrics(&cpumon);

        int margin = 1;
        int cpu_x = margin;
        int cpu_y = margin;
        int cpu_w = term_w;
        int cpu_h = term_h;
        if (redraw)
            p = render_static_interface(p, cpu_x, cpu_y, cpu_w, cpu_h);

        p = render_interface(&cpumon, p, cpu_x, cpu_y, cpu_w, cpu_h);

        if (write(STDOUT_FILENO, buf, p - buf) == -1)
            perror("write failed");

        usleep(delay);
    }

    cleanup_cpumon(&cpumon);
    tui_restore();

    return 0;
}
