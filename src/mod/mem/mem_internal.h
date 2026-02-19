#ifndef MEM_INTERNAL_H
#define MEM_INTERNAL_H

#include <stdint.h>
#include "mod/mem/mem.h"

void parse_mem(MemMon memmon[static 1]);
void draw_meter(char* p[static 1], int x, int y, int w, const char *label, const char *color_bar, int perc, uint64_t val_kb);

#endif
