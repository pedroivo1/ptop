#ifndef MEM_INTERNAL_H
#define MEM_INTERNAL_H

#include "mod/mem/mem.h"
#include <stddef.h>
#include <stdint.h>

void parse_mem(MemMon memmon[static 1]);
void draw_meter(char* p[static 1],
                size_t x,
                size_t y,
                size_t w,
                const char* label,
                const char* color_bar,
                unsigned perc,
                uint64_t val_kb);

#endif
