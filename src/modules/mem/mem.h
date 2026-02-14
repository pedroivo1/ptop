#ifndef MEM_H
#define MEM_H

#include "common/rect.h"

typedef struct
{
    uint64_t total;
    uint64_t free;
    uint64_t available;
    uint64_t cached;
    uint64_t used;

    int fd_mem;

    Rect rect;

} MemMon;

void init_mem(MemMon *memmon);
void deinit_mem(MemMon *memmon);
void update_mem_data(MemMon *memmon);

char *draw_mem_ui(MemMon *memmon, char *p);
char *draw_mem_data(MemMon *memmon, char *p);

#endif
