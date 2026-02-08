#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdalign.h>
#include <poll.h>
#include "cfg.h"
#include "modules/cpu.h"
#include "modules/mem.h"
#include "tui.h"

volatile sig_atomic_t run = 1;

void handle_sigint(int sig)
{
    (void)sig;
    run = 0;
}

int main()
{
    signal(SIGINT, handle_sigint);

    CpuMon cpumon;
    init_cpu(&cpumon);

    MemMon memmon;
    init_mem(&memmon);

    static alignas(64) char buf[OUT_BUFF_LEN];

    int redraw = 1;
    tui_setup(BG_BLACK, WHITE);

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    while (run)
    {
        char *p = buf;
        p = tui_begin_frame(p, &redraw);

        update_cpu_data(&cpumon);
        update_mem_data(&memmon);

        int margin = 1;
        // int split_h = (term_h * 60) / 100;
        int split_h = term_h - 8;

        // --- BLOCO DA CPU (Topo) ---
        int cpu_x = margin;
        int cpu_y = margin;
        int cpu_w = term_w; 
        int cpu_h = split_h;
        if (redraw)
            p = draw_cpu_ui(p, cpu_x, cpu_y, cpu_w, cpu_h);
        p = draw_cpu_data(&cpumon, p, cpu_x, cpu_y, cpu_w, cpu_h);

        // --- BLOCO DA MEMÓRIA (Resto abaixo) ---
        int mem_x = margin;
        int mem_y = cpu_y + cpu_h; 
        int mem_w = term_w;
        int mem_h = term_h - cpu_h - margin; 
        if (redraw)
            p = draw_mem_ui(p, mem_x, mem_y, mem_w, mem_h);
        p = draw_mem_data(&memmon, p, mem_x, mem_y, mem_w, mem_h);

        if (write(STDOUT_FILENO, buf, p - buf) == -1)
            perror("write failed");

        int ret = poll(fds, 1, DELAY_MS); 
    
        if (ret > 0 && (fds[0].revents & POLLIN))
        {
            char key;
            if (read(STDIN_FILENO, &key, 1) > 0)
            {
                if (key == 'q') run = 0;
            }
        }
    }
    deinit_cpu(&cpumon);
    tui_restore();

    return 0;
}
