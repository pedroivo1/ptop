#include <unistd.h>

#include "util/util.h"
#include "modules/mem/mem.h"
#include "ui/ui.h"
#include "cfg/buf.h"

void parse_mem(MemMon *memmon)
{
    static char buf[MEM_STAT_BUF_LEN];
    ssize_t bytes_read = pread(memmon->fd_mem, buf, sizeof(buf) - 1, 0);
    if (bytes_read < 0) return;
    buf[bytes_read] = '\0';

    char *p = buf;

    skip_to_digit(&p);
    memmon->total = str_to_uint64(&p);

    skip_to_digit(&p);
    memmon->free = str_to_uint64(&p);

    skip_to_digit(&p);
    memmon->available = str_to_uint64(&p);

    skip_line(&p);
    skip_line(&p);
    skip_to_digit(&p);
    memmon->cached = str_to_uint64(&p);

    if (memmon->total > 0)
        memmon->used = memmon->total - memmon->available;
}
