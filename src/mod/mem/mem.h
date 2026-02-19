#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include "util/rect.h"

typedef struct {
    uint64_t total;
    uint64_t free;
    uint64_t available;
    uint64_t cached;
    uint64_t used;

    int fd_mem;

    Rect rect;

} MemMon;

void init_mem(MemMon memmon[static 1]);
void deinit_mem(MemMon memmon[static 1]);
void update_mem_data(MemMon memmon[static 1]);

void draw_mem_ui(MemMon memmon[static 1], char* p[static 1]);
void draw_mem_data(MemMon memmon[static 1], char* p[static 1]);

#endif
