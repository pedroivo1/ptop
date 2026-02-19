#include <signal.h>
#include <stdlib.h>
#include "app/app.h"

volatile sig_atomic_t g_signal_quit = 0;

void handle_sigint(int sig) {
    (void)sig;
    g_signal_quit = 1;
}

int main() {
    signal(SIGINT, handle_sigint);

    AppContext ctx;
    app_init(&ctx);
    app_run(&ctx);
    app_destroy(&ctx);

    return EXIT_SUCCESS;
}
