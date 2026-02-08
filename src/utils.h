#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define APPEND_LIT(buf, str) (memcpy(buf, str, sizeof(str)-1), buf + sizeof(str)-1)

static inline uint64_t str_to_uint64(char **p)
{
    uint64_t val = 0;
    while (**p >= '0' && **p <= '9')
    {
        val = (val * 10) + (**p - '0');
        (*p)++;
    }
    return val;
}

static inline void skip_to_digit(char **p)
{
    while (**p && (**p < '0' || **p > '9'))
        (*p)++;
}

static inline void skip_line(char **p)
{
    while (**p && (**p != '\n'))
        (*p)++;

    if (**p)
        (*p)++;
}

static inline uint64_t read_sysfs_uint64(int fd)
{
    char buf[32];
    uint64_t val = 0;

    ssize_t bytes_read = pread(fd, buf, sizeof(buf) - 1, 0);
    if (bytes_read > 0)
    {
        buf[bytes_read] = '\0'; 
        char *ptr = buf; 

        val = str_to_uint64(&ptr);
    }
    return val;
}

static const char g_digits_lut[200] = 
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

static inline char *append_u64_base(char *buf, uint64_t val)
{
    if (val == 0)
    {
        *buf++ = '0';
        return buf;
    }

    char temp[24]; 
    char *p = temp + 24;

    while (val >= 100)
    {
        unsigned int index = (val % 100) * 2;
        val /= 100;
        p -= 2;
        memcpy(p, &g_digits_lut[index], 2);
    }

    if (val < 10) *--p = val + '0';
    else
    {
        unsigned int index = val * 2;
        p -= 2;
        memcpy(p, &g_digits_lut[index], 2);
    }

    size_t len = (temp + 24) - p;
    memcpy(buf, p, len);
    return buf + len;
}

static inline char *append_i64_base(char *buf, int64_t val)
{
    if (val < 0)
    {
        *buf++ = '-';
        return append_u64_base(buf, (uint64_t)-(val));
    }
    return append_u64_base(buf, (uint64_t)val);
}

#define append_num(buf, val) _Generic((val), \
    _Bool:              append_u64_base, \
    unsigned char:      append_u64_base, \
    unsigned short:     append_u64_base, \
    unsigned int:       append_u64_base, \
    unsigned long:      append_u64_base, \
    unsigned long long: append_u64_base, \
    signed char:        append_i64_base, \
    short:              append_i64_base, \
    int:                append_i64_base, \
    long:               append_i64_base, \
    long long:          append_i64_base  \
)(buf, val)

static inline char *append_fixed(char *buf, uint64_t val, uint64_t divisor, uint64_t prec_mult)
{
    buf = append_u64_base(buf, val / divisor);

    if (prec_mult <= 1) return buf;

    *buf++ = '.';

    uint64_t frac = ((val % divisor) * prec_mult) / divisor;
    uint64_t check = prec_mult / 10;
    while (check > 1)
    {
        if (frac < check) *buf++ = '0';
        check /= 10;
    }

    return append_u64_base(buf, frac);
}

static inline char *append_fixed2(char *buf, uint64_t val, uint64_t shift, uint64_t prec_mult)
{
    buf = append_u64_base(buf, val >> shift);

    if (prec_mult <= 1) return buf;

    *buf++ = '.';

    uint64_t mask = ((uint64_t)1 << shift) - 1;
    uint64_t remainder = val & mask;

    uint64_t frac = (remainder * prec_mult) >> shift;

    uint64_t check = prec_mult / 10;
    while (check > 1)
    {
        if (frac < check) *buf++ = '0';
        check /= 10;
    }

    return append_u64_base(buf, frac);
}

static inline char *append_str(char *buf, const char *str)
{
    size_t len = strlen(str);
    memcpy(buf, str, len);
    return buf + len;
}

#endif
