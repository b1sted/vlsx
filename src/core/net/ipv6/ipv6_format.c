/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv6/ipv6_format.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "core/net/ipv6/ipv6_types.h"

static bool ipv6_append_text(char *dest, size_t dest_size, size_t *position, const char *text);
static bool ipv6_append_char(char *dest, size_t dest_size, size_t *position, char ch);
static bool ipv6_append_hextet(char *dest, size_t dest_size, size_t *position, uint16_t value,
                               bool padded);

static bool ipv6_append_text(char *dest, size_t dest_size, size_t *position, const char *text)
{
    const size_t length = strlen(text);

    if (*position + length >= dest_size) {
        return false;
    }

    memcpy(dest + *position, text, length);
    *position += length;
    dest[*position] = '\0';

    return true;
}

static bool ipv6_append_char(char *dest, size_t dest_size, size_t *position, char ch)
{
    if (*position + 1 >= dest_size) {
        return false;
    }

    dest[*position] = ch;
    (*position)++;
    dest[*position] = '\0';

    return true;
}

static bool ipv6_append_hextet(char *dest, size_t dest_size, size_t *position, uint16_t value,
                               bool padded)
{
    const int written = snprintf(dest + *position, dest_size - *position, padded ? "%04x" : "%x",
                                 (unsigned int)value);

    if (written < 0 || (size_t)written >= dest_size - *position) {
        return false;
    }

    *position += (size_t)written;
    return true;
}

void ipv6_format_no_leading_zeros(const ipv6_addr_t *addr, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (i > 0 && !ipv6_append_char(dest, dest_size, &position, ':')) {
            dest[0] = '\0';
            return;
        }

        if (!ipv6_append_hextet(dest, dest_size, &position, addr->hextets[i], false)) {
            dest[0] = '\0';
            return;
        }
    }
}

void ipv6_format_compressed(const ipv6_addr_t *addr, char *dest, size_t dest_size)
{
    size_t position = 0;
    size_t best_start = 0;
    size_t best_length = 0;
    size_t current_start = 0;
    size_t current_length = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (addr->hextets[i] == 0) {
            if (current_length == 0) {
                current_start = i;
            }

            current_length++;

            if (current_length > best_length) {
                best_start = current_start;
                best_length = current_length;
            }
        } else {
            current_length = 0;
        }
    }

    if (best_length < 2) {
        best_length = 0;
    }

    if (best_length == 0) {
        for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
            if (i > 0 && !ipv6_append_char(dest, dest_size, &position, ':')) {
                dest[0] = '\0';
                return;
            }

            if (!ipv6_append_hextet(dest, dest_size, &position, addr->hextets[i], false)) {
                dest[0] = '\0';
                return;
            }
        }

        return;
    }

    for (size_t i = 0; i < best_start; i++) {
        if (i > 0 && !ipv6_append_char(dest, dest_size, &position, ':')) {
            dest[0] = '\0';
            return;
        }

        if (!ipv6_append_hextet(dest, dest_size, &position, addr->hextets[i], false)) {
            dest[0] = '\0';
            return;
        }
    }

    if (!ipv6_append_text(dest, dest_size, &position, "::")) {
        dest[0] = '\0';
        return;
    }

    const size_t right_start = best_start + best_length;

    for (size_t i = right_start; i < IPV6_HEXTET_COUNT; i++) {
        if (i > right_start && !ipv6_append_char(dest, dest_size, &position, ':')) {
            dest[0] = '\0';
            return;
        }

        if (!ipv6_append_hextet(dest, dest_size, &position, addr->hextets[i], false)) {
            dest[0] = '\0';
            return;
        }
    }
}

void ipv6_format_expanded(const ipv6_addr_t *addr, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    for (size_t i = 0; i < IPV6_HEXTET_COUNT; i++) {
        if (i > 0 && !ipv6_append_char(dest, dest_size, &position, ':')) {
            dest[0] = '\0';
            return;
        }

        if (!ipv6_append_hextet(dest, dest_size, &position, addr->hextets[i], true)) {
            dest[0] = '\0';
            return;
        }
    }
}

void ipv6_format_interface_id(const ipv6_addr_t *interface_id, uint8_t prefix, char *dest,
                              size_t dest_size)
{
    size_t position = 0;
    size_t start = (size_t)prefix / 16;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (interface_id == NULL) {
        return;
    }

    if (start > IPV6_HEXTET_COUNT) {
        start = IPV6_HEXTET_COUNT;
    }

    for (size_t i = start; i < IPV6_HEXTET_COUNT; i++) {
        if (i > start && !ipv6_append_char(dest, dest_size, &position, ':')) {
            dest[0] = '\0';
            return;
        }

        if (!ipv6_append_hextet(dest, dest_size, &position, interface_id->hextets[i], true)) {
            dest[0] = '\0';
            return;
        }
    }
}
