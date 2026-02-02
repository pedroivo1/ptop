#include "cfg.h"
#include "monitor.h"
#include "display.h"
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t run = 1;

void handle_sigint(int sig) {
    run = 0;
}

int main() {
    signal(SIGINT, handle_sigint);

    CpuMonitor cpumon;
    init_cpumon(&cpumon);
    
    int delay = DELAY_MS * 1000;

    setup_terminal();
    int i = 0;
    while (run) {
        i++;
        if (i > 10) run = 0;
        update_metrics(&cpumon);
        render_interface(&cpumon);
        usleep(delay);
    }

    cleanup_cpumon(&cpumon);
    restore_terminal();

    return 0;
}
