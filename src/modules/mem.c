#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "mem.h"
#include "../cfg.h"
#include "../utils.h"
#include "../tui.h"

#include <inttypes.h>

#define C_LABEL   "\033[38;5;250m"
#define C_VALUE   "\033[38;5;255m"
#define C_BAR_BG  "\033[38;5;236m"

#define C_USED    "\033[38;5;160m"
#define C_AVAIL   "\033[38;5;112m"
#define C_CACHE   "\033[38;5;214m"
#define C_FREE    "\033[38;5;243m"
#define MEM_BORDER_C "\033[38;5;101m"

void init_mem(MemMon *memmon)
{
    memset(memmon, 0, sizeof(*memmon));
    memmon->fd_mem = open(MEM_PATH, O_RDONLY);
}

void deinit_mem(MemMon *memmon)
{
    close(memmon->fd_mem);
}

static void parse_mem(MemMon *memmon)
{
    static char buf[MEM_BUFF_LEN];
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

void update_mem_data(MemMon *memmon)
{
    parse_mem(memmon);
}

static char *draw_meter(char *p, int x, int y, int w, const char *label, const char *color_bar, int perc, uint64_t val_kb)
{
    int label_w = 12;
    int value_w = 11;
    int bar_w = w - label_w - value_w;
    if (bar_w < 5) bar_w = 5;

    p = tui_at(p, x, y);

    p = append_str(p, C_LABEL);
    p = append_str(p, label);
    APPEND_LIT(&p, ":");

    if (perc < 100) APPEND_LIT(&p, " ");
    if (perc < 10)  APPEND_LIT(&p, " ");

    p = append_str(p, color_bar);
    append_num(&p, perc);
    APPEND_LIT(&p, "%");
    APPEND_LIT(&p, " ");

    int fill = (bar_w * perc) / 100;
    for (int i = 0; i < fill; i++) *p++ = '|';
    p = append_str(p, C_BAR_BG);
    for (int i = fill; i < bar_w; i++) *p++ = '|';

    p = append_str(p, C_VALUE);
    APPEND_LIT(&p, " ");
    p = append_fixed2(p, val_kb, 20, 100);
    if (val_kb < 10485760) APPEND_LIT(&p, " ");
    APPEND_LIT(&p, " GiB");

    return p;
}

char *draw_mem_ui(char *p, int x, int y, int w, int h)
{
    // --- MAIN BOX ---
    p = tui_draw_box(p, x, y, w, h, MEM_BORDER_C);
    p = append_str(p, WHITE);

    return p;
}

char *draw_mem_data(MemMon *memmon, char *p, int x, int y, int w, int h)
{
    (void)h;
    int start_x = x + 2;
    int current_y = y + 1;
    int inner_w = w - 4;

    p = tui_at(p, start_x, current_y++);
    p = append_str(p, "\033[1mRAM\033[22m Total: ");

    p = append_fixed2(p, memmon->total, 20, 100);
    p = append_str(p, " GiB");

    if (memmon->total == 0) return p;

    int p_used   = (memmon->used   * 100) / memmon->total;
    int p_avail  = (memmon->available * 100) / memmon->total;
    int p_cached = (memmon->cached * 100) / memmon->total;
    int p_free   = (memmon->free   * 100) / memmon->total;

    p = draw_meter(p, start_x, current_y++, inner_w, "Used ", C_USED,  p_used,   memmon->used);
    p = draw_meter(p, start_x, current_y++, inner_w, "Avail", C_AVAIL, p_avail,  memmon->available);
    p = draw_meter(p, start_x, current_y++, inner_w, "Cache", C_CACHE, p_cached, memmon->cached);
    p = draw_meter(p, start_x, current_y++, inner_w, "Free ", C_FREE,  p_free,   memmon->free);

    return p;
}
