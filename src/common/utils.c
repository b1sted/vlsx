/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "common/utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

uint32_t util_round_up_pow2(uint32_t value)
{
    if (value == 0) {
        return 1;
    }

    if (value > UINT32_C(2147483648)) {
        return 0;
    }

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;

    return value + 1;
}

uint8_t util_pow2_exponent(uint32_t value)
{
    static const uint8_t de_bruijn_32[32] = {0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                             15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                             16, 7,  26, 12, 18, 6,  11, 5,  10, 9};

    const uint32_t hash = (uint32_t)(value * UINT32_C(0x077CB531));

    return de_bruijn_32[hash >> 27];
}

void util_uint8_itoa(uint8_t number, char *dest)
{
    char *cursor = dest + 3;

    *cursor = '\0';

    if (number == 0) {
        *(--cursor) = '0';
        return;
    }

    while (number > 0) {
        *(--cursor) = (char)((number % 10) + '0');
        number = (uint8_t)(number / 10);
    }
}

int util_count_digits(uint32_t number)
{
    if (number >= 100000) {
        if (number >= 10000000) {
            if (number >= 100000000) {
                if (number >= 1000000000) {
                    return 10;
                }

                return 9;
            }

            return 8;
        }

        if (number >= 1000000) {
            return 7;
        }

        return 6;
    }

    if (number >= 1000) {
        if (number >= 10000) {
            return 5;
        }

        return 4;
    }

    if (number >= 100) {
        return 3;
    }

    if (number >= 10) {
        return 2;
    }

    return 1;
}

bool util_string_combine(const char *first, const char *separator, const char *second, char *dest,
                         size_t dest_size)
{
    if (first == NULL || separator == NULL || second == NULL || dest == NULL || dest_size == 0) {
        return false;
    }

    const size_t first_len = strlen(first);
    const size_t separator_len = strlen(separator);
    const size_t second_len = strlen(second);
    const size_t total_len = first_len + separator_len + second_len;

    if (total_len >= dest_size) {
        return false;
    }

    memcpy(dest, first, first_len);
    memcpy(dest + first_len, separator, separator_len);
    memcpy(dest + first_len + separator_len, second, second_len);
    dest[total_len] = '\0';

    return true;
}
