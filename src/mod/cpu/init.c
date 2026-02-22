#include "cfg/path.h"
#include "mod/cpu/cpu.h"
#include "mod/cpu/cpu_internal.h"
#include "util/util.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_RETRIES    10
#define RETRY_DELAY_MS 10

void malloc_cpu(CpuMon cpumon[static 1])
{
   size_t num_lines = cpumon->thread_count + 2;
   size_t line_len = 512;
   cpumon->stat_buffer_size = num_lines * line_len;
   cpumon->stat_buffer = malloc(cpumon->stat_buffer_size);

   cpumon->prev_total =
      calloc(cpumon->thread_count, sizeof(*cpumon->prev_total));
   cpumon->prev_idle = calloc(cpumon->thread_count, sizeof(*cpumon->prev_idle));
   cpumon->usage = calloc(cpumon->thread_count, sizeof(*cpumon->usage));
   cpumon->fd_freq = malloc(cpumon->phy_count * sizeof(*cpumon->fd_freq));

   cpumon->graph_hist =
      calloc(cpumon->thread_count * cpumon->graph_width, sizeof(uint8_t));
}

void open_fd(int fd[static 1], char const* path)
{
   int retries = 0;
   int tmp_fd = -1;
   do
   {
      tmp_fd = open(path, O_RDONLY);

      if (tmp_fd > STDERR_FILENO)
         break;

      usleep(RETRY_DELAY_MS * 1000);
      retries++;

   } while (retries < MAX_RETRIES);

   *fd = tmp_fd;
}

void temp_path(char path[static 1])
{
   uint64_t hwmon_cpu_id;
   do
   {
      hwmon_cpu_id = get_temp_id();
   } while (hwmon_cpu_id == UINT64_MAX);

   APPEND_LIT(&path, HWMON_DIR HWMON);
   append_num(&path, hwmon_cpu_id);
   APPEND_LIT(&path, HWMON_TEMP);
   append_num(&path, 1);
   APPEND_LIT(&path, HWMON_INPUT);
   *path = '\0';
}

void freq_path(char path[static 1], int id)
{
   APPEND_LIT(&path, SYS_CPU_BASE);
   append_num(&path, id);
   APPEND_LIT(&path, CPU_FREQ_FILE);
   *path = '\0';
}

void open_cpu_fds(CpuMon cpumon[static 1])
{
   open_fd(&cpumon->fd_stat, STAT);

   char path[64];

   temp_path(path);
   open_fd(&cpumon->fd_temp, path);

   for (size_t i = 0; i < cpumon->phy_count; i++)
   {
      freq_path(path, i);
      open_fd(&cpumon->fd_freq[i], path);
   }
}

void init_cpu(CpuMon cpumon[static 1])
{
   memset(cpumon, 0, sizeof(*cpumon));
   cpumon->graph_width = 21;

   get_topology(cpumon);
   malloc_cpu(cpumon);
   open_cpu_fds(cpumon);
}
