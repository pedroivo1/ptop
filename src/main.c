#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdalign.h>
#include <time.h>
#include <stdlib.h>

#include "app/app.h"
#include "common/utils.h"
#include "ui/ui.h"
#include "common/cfg.h"

volatile sig_atomic_t g_signal_quit = 0;

void handle_sigint(int sig)
{
    (void)sig;
    g_signal_quit = 1;
}

int main()
{
    signal(SIGINT, handle_sigint);

    AppContext ctx;
    app_init(&ctx);
    app_run(&ctx);
    app_destroy(&ctx);

    return 0;
}
