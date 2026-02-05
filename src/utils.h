#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdint.h>

#define APPEND_LIT(buf, str) (memcpy(buf, str, sizeof(str)-1), buf + sizeof(str)-1)

static const char g_digits_lut[200] = 
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

static inline char *append_int(char *buf, int val)
{
    if (val == 0)
    {
        *buf++ = '0';
        return buf;
    }

    unsigned int uval = val;
    if (val < 0)
    {
        *buf++ = '-';
        uval = 0 - val;
    }

    char temp[12];
    char *p = temp + 12;

    while (uval >= 100)
    {
        unsigned int index = (uval % 100) * 2;
        uval /= 100;
        p -= 2;
        memcpy(p, &g_digits_lut[index], 2);
    }

    if (uval < 10)
    {
        *--p = uval + '0';
    }
    else
    {
        unsigned int index = uval * 2;
        p -= 2;
        memcpy(p, &g_digits_lut[index], 2);
    }

    size_t len = (temp + 12) - p;
    memcpy(buf, p, len);

    return buf + len;
}

static inline char *append_str(char *buf, const char *str)
{
    size_t len = strlen(str);
    memcpy(buf, str, len);
    return buf + len;
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
