#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"
#include "../cfg.h"
#include "../utils.h"

#include <inttypes.h>

void init_memmon(MemMonitor *memmon)
{
    memset(memmon, sizeof(*memmon), 0);

    memmon->fd_mem = open(MEM_PATH, O_RDONLY);
}

void parse_mem(MemMonitor *memmon)
{
    static char buf[MEM_BUFF_LEN];
    char *p = buf;

    ssize_t bytes_read = pread(memmon->fd_mem, buf, sizeof(buf) - 1, 0);
    if (bytes_read < 0) return;

    buf[bytes_read] = '\0';

    skip_to_digit(&p);
    memmon->mem_total = str_to_int(&p);

    skip_to_digit(&p);
    memmon->mem_free = str_to_int(&p);
}

int main()
{
    MemMonitor memmon;
    init_memmon(&memmon);
    parse_mem(&memmon);

    printf("fd: %d\n", memmon.fd_mem);
    printf("total: %" PRIu64 "\n", memmon.mem_total);
    printf("free: %" PRIu64 "\n", memmon.mem_free);
}
