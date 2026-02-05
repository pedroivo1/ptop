#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdint.h>

#define APPEND_LIT(buf, str) (memcpy(buf, str, sizeof(str)-1), buf + sizeof(str)-1)

static inline char *append_int(char *buf, int val)
{
    char temp[16];
    char *p = temp;

    do
    {
        *p++ = (val % 10) + '0';
        val /= 10;
    } while ( val > 0);

    while (temp < p)
    {
        *buf++ = *--p;
    }

    return buf;
}

static inline char *append_str(char *buf, const char *str)
{
    while (*str) *buf++ = *str++;
    return buf;
}

static inline int read_sysfs_int(int fd)
{
    char buf[16];
    int val = 0;
    ssize_t bytes_read = pread(fd, buf, sizeof(buf) - 1, 0);
    if (bytes_read > 0)
    {
        char *p = buf;
        while (*p >= '0' && *p <= '9') val = (val * 10) + (*p++ - '0');
    }
    return val;
}

#endif
