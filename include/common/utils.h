/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_UTILS_H
#define VLSX_COMMON_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t util_round_up_pow2(uint32_t value);
uint8_t util_pow2_exponent(uint32_t value);

void util_uint8_itoa(uint8_t number, char *dest);

int util_count_digits(uint32_t number);

bool util_string_combine(const char *first, const char *separator, const char *second, char *dest,
                         size_t dest_size);

#endif /* VLSX_COMMON_UTILS_H */
