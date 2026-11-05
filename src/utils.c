/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"

uint32_t round_up_pow2(uint32_t n) {
    if (n == 0)
        return 1;
    if (n > 2147483648U)
        return 0;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    return n + 1;
}

uint8_t get_pow2_exponent(uint32_t n) {
    static const uint8_t de_bruijn_32[32] = {
        0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};

    uint32_t hash = (uint32_t)(n * 0x077CB531U);
    return de_bruijn_32[hash >> 27];
}

void uint8_itoa(uint8_t num, char *dest) {
    char *p = dest + 3;
    *p = '\0';

    if (num == 0) {
        *(--p) = '0';
        return;
    }

    while (num > 0) {
        *(--p) = (num % 10) + '0';
        num /= 10;
    }
}

int count_digits(uint32_t num) {
    if (num >= 100000) {
        if (num >= 10000000) {
            if (num >= 100000000) {
                if (num >= 1000000000)
                    return 10;
                return 9;
            }
            return 8;
        }
        if (num >= 1000000)
            return 7;
        return 6;
    } else {
        if (num >= 1000) {
            if (num >= 10000)
                return 5;
            return 4;
        }
        if (num >= 100)
            return 3;
        if (num >= 10)
            return 2;
        return 1;
    }
}

bool string_combine(const char *first_str, const char *sep,
                    const char *second_str, char *dest,
                    const size_t dest_size) {
    size_t first_len = strlen(first_str);
    size_t sep_len = strlen(sep);
    size_t second_len = strlen(second_str);

    if (first_len + sep_len + second_len > dest_size) {
        return false;
    }

    memcpy(dest, first_str, first_len);
    memcpy(dest + first_len, sep, sep_len);
    memcpy(dest + first_len + sep_len, second_str, second_len);
    dest[first_len + sep_len + second_len] = '\0';

    return true;
}
