#include "app/app.h"
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t g_signal_quit = 0;

void handle_sigint(int sig)
{
   (void)sig;
   g_signal_quit = 1;
}

int main()
{
   signal(SIGINT, handle_sigint);

   CpuMon cpu;
   MemMon mem;
   unsigned delay = 200;

   app_init(&cpu, &mem);
   app_run(&cpu, &mem, &delay);
   app_destroy(&cpu, &mem);

   return EXIT_SUCCESS;
}
