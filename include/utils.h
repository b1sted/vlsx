/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t round_up_pow2(uint32_t n);
uint8_t get_pow2_exponent(uint32_t n);

void uint8_itoa(uint8_t num, char *dest);

int count_digits(uint32_t num);

bool string_combine(const char *first_str, const char *sep,
                    const char *second_str, char *dest, const size_t dest_size);

#endif /* UTILS_H */
