#include <stdint.h>

typedef struct
{
    uint64_t mem_total;
    uint64_t mem_free;
    uint64_t mem_available;
    uint64_t mem_cached;

    int fd_mem;

} MemMonitor;
// MEM_BUFF_LEN
void init_memmon(MemMonitor *memmon);
