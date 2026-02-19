#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define APPEND_LIT(buf, str) ((void)(memcpy(*buf, str, sizeof(str) - 1), *buf += sizeof(str) - 1))

static inline uint64_t current_time_ms() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static inline uint64_t str_to_uint64(char **p) {
    uint64_t val = 0;
    while (**p >= '0' && **p <= '9') {
        val = (val * 10) + (**p - '0');
        (*p)++;
    }
    return val;
}

static inline void skip_to_digit(char **p) {
    while (**p && (**p < '0' || **p > '9')) {
        (*p)++;
    }
}

static inline void skip_line(char **p) {
    while (**p && (**p != '\n')) {
        (*p)++;
    }

    if (**p) {
        (*p)++;
    }
}

static inline uint64_t read_sysfs_uint64(int fd) {
    char buf[64];
    ssize_t bytes_read = pread(fd, buf, sizeof(buf) - 1, 0);

    if (bytes_read > 0) {
        buf[bytes_read] = '\0';
        char *ptr = buf;
        return str_to_uint64(&ptr);
    }
    return 0;
}

static const char g_digits_lut[200] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

static inline void append_u64_base(char **buf, uint64_t val) {
    if (val == 0) {
        *(*buf)++ = '0';
        return;
    }

    char temp[24];
    char *p = temp + 24;

    while (val >= 100) {
        unsigned int index = (val % 100) << 1;
        val /= 100;
        p -= 2;
        p[0] = g_digits_lut[index];
        p[1] = g_digits_lut[index + 1];
    }

    if (val < 10) {
        *--p = val + '0';
    } else {
        unsigned int index = val << 1;
        p -= 2;
        p[0] = g_digits_lut[index];
        p[1] = g_digits_lut[index + 1];
    }

    size_t len = (temp + 24) - p;
    memcpy(*buf, p, len);
    *buf += len;
}

static inline void append_i64_base(char **buf, int64_t val) {
    if (val < 0) {
        *(*buf)++ = '-';
        append_u64_base(buf, (uint64_t)(-val));
    } else {
        append_u64_base(buf, (uint64_t)val);
    }
}

#define append_num(buf, val) _Generic((val), \
    _Bool:              append_u64_base, \
    unsigned char:      append_u64_base, \
    unsigned short:     append_u64_base, \
    unsigned int:       append_u64_base, \
    unsigned long:      append_u64_base, \
    unsigned long long: append_u64_base, \
    char:               append_i64_base, \
    signed char:        append_i64_base, \
    short:              append_i64_base, \
    int:                append_i64_base, \
    long:               append_i64_base, \
    long long:          append_i64_base  \
)(buf, val)

static inline void append_two_digits(char **p, int val) {
    if (val < 10) {
        *(*p)++ = '0';
        *(*p)++ = (char)('0' + val);
    } else {
        *(*p)++ = (char)('0' + (val / 10));
        *(*p)++ = (char)('0' + (val % 10));
    }
}

static inline void append_str(char **buf, const char *str) {
    size_t len = strlen(str);
    memcpy(*buf, str, len);
    *buf += len;
}

static inline void append_fixed_1d(char **buf, uint64_t val, uint64_t divisor) {
    append_u64_base(buf, val / divisor);
    *(*buf)++ = '.';

    uint64_t remainder = val % divisor;
    uint64_t digit = (remainder * 10) / divisor;

    *(*buf)++ = (char)('0' + digit);
}

static inline void append_fixed_2d(char **buf, uint64_t val, uint64_t divisor) {
    append_u64_base(buf, val / divisor);
    *(*buf)++ = '.';

    uint64_t remainder = val % divisor;
    uint64_t frac = (remainder * 100) / divisor;

    if (frac < 10)
        *(*buf)++ = '0';

    append_u64_base(buf, frac);
}

static inline void append_fixed_shift_1d(char **buf, uint64_t val, uint64_t shift) {
    append_u64_base(buf, val >> shift);
    *(*buf)++ = '.';

    uint64_t mask = ((uint64_t)1 << shift) - 1;
    uint64_t digit = ((val & mask) * 10) >> shift;

    *(*buf)++ = (char)('0' + digit);
}

static inline void append_fixed_shift_2d(char **buf, uint64_t val, uint64_t shift) {
    append_u64_base(buf, val >> shift);
    *(*buf)++ = '.';

    uint64_t mask = ((uint64_t)1 << shift) - 1;

    uint64_t frac = ((val & mask) * 100) >> shift;

    if (frac < 10)
        *(*buf)++ = '0';
    append_u64_base(buf, frac);
}

static inline void append_fixed_generic(char **buf, uint64_t val, uint64_t divisor, uint64_t prec_mult) {
    append_u64_base(buf, val / divisor);

    if (prec_mult <= 1) {
        return;
    }

    *(*buf)++ = '.';

    uint64_t remainder = val % divisor;

    while (prec_mult > 1) {
        prec_mult /= 10;

        remainder *= 10;
        uint64_t digit = remainder / divisor;
        remainder %= divisor;

        *(*buf)++ = (char)('0' + digit);
    }
}

static inline void append_fixed_shift_generic(char **buf, uint64_t val, uint64_t shift, uint64_t prec_mult)
{
    append_u64_base(buf, val >> shift);

    if (prec_mult <= 1) {
        return;
    }

    *(*buf)++ = '.';

    uint64_t mask = ((uint64_t)1 << shift) - 1;
    uint64_t remainder = val & mask;

    uint64_t frac = (remainder * prec_mult) >> shift;

    if (prec_mult >= 10) {
        if (frac < prec_mult / 10) {
            *(*buf)++ = '0';
        }
        if (prec_mult >= 100) {
            if (frac < prec_mult / 100) {
                *(*buf)++ = '0';
            }
            if (prec_mult >= 1000) {
                if (frac < prec_mult / 1000) {
                    *(*buf)++ = '0';
                }
            }
        }
    }
    append_u64_base(buf, frac);
}

#endif
