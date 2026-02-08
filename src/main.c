#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdalign.h>
#include <time.h>

#include "app.h"
#include "utils.h"
#include "tui.h"
#include "cfg.h"

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

    tui_setup(BG_BLACK, WHITE);
    
    static alignas(64) char buf[OUT_BUFF_LEN];

    uint64_t last_update_time = 0;
    uint64_t now;

    while (ctx.running)
    {
        now = current_time_ms();

        int is_tick = (last_update_time == 0 || now - last_update_time >= DELAY_MS);

        if (is_tick)
        {
            last_update_time = now;
            app_update_state(&ctx);
        }

        if (is_tick || ctx.needs_resize || ctx.force_redraw)
        {
            int bytes_written = app_render_frame(&ctx, buf);

            if (bytes_written > 0)
                if (write(STDOUT_FILENO, buf, bytes_written) == -1) break;
        }

        int time_spent = (int)(current_time_ms() - last_update_time);
        int time_to_wait = DELAY_MS - time_spent;
        if (time_to_wait < 0) time_to_wait = 0;

        handle_input(&ctx, time_to_wait);
    }

    app_destroy(&ctx);
    tui_restore();
    return 0;
}
