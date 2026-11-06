/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/mac/mac_format.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "core/net/mac/mac_types.h"

static bool mac_append_char(char *dest, size_t dest_size, size_t *position, char ch);

static bool mac_append_char(char *dest, size_t dest_size, size_t *position, char ch)
{
    if (*position + 1 >= dest_size) {
        return false;
    }

    dest[*position] = ch;
    (*position)++;
    dest[*position] = '\0';

    return true;
}

void mac_format_colon(const mac_addr_t *addr, char *dest, size_t dest_size)
{
    int written = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    written = snprintf(dest, dest_size, "%02X:%02X:%02X:%02X:%02X:%02X",
                       (unsigned int)addr->octets[0], (unsigned int)addr->octets[1],
                       (unsigned int)addr->octets[2], (unsigned int)addr->octets[3],
                       (unsigned int)addr->octets[4], (unsigned int)addr->octets[5]);

    if (written < 0 || (size_t)written >= dest_size) {
        dest[0] = '\0';
    }
}

void mac_format_cisco(const mac_addr_t *addr, char *dest, size_t dest_size)
{
    int written = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    written = snprintf(dest, dest_size, "%02X%02X.%02X%02X.%02X%02X", (unsigned int)addr->octets[0],
                       (unsigned int)addr->octets[1], (unsigned int)addr->octets[2],
                       (unsigned int)addr->octets[3], (unsigned int)addr->octets[4],
                       (unsigned int)addr->octets[5]);

    if (written < 0 || (size_t)written >= dest_size) {
        dest[0] = '\0';
    }
}

void mac_format_oui(const mac_addr_t *addr, char *dest, size_t dest_size)
{
    int written = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    written = snprintf(dest, dest_size, "%02X:%02X:%02X", (unsigned int)addr->octets[0],
                       (unsigned int)addr->octets[1], (unsigned int)addr->octets[2]);

    if (written < 0 || (size_t)written >= dest_size) {
        dest[0] = '\0';
    }
}

void mac_format_nic(const mac_addr_t *addr, char *dest, size_t dest_size)
{
    int written = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    written = snprintf(dest, dest_size, "%02X:%02X:%02X", (unsigned int)addr->octets[3],
                       (unsigned int)addr->octets[4], (unsigned int)addr->octets[5]);

    if (written < 0 || (size_t)written >= dest_size) {
        dest[0] = '\0';
    }
}

void mac_format_binary(const mac_addr_t *addr, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    if (addr == NULL) {
        return;
    }

    for (size_t i = 0; i < MAC_OCTET_COUNT; i++) {
        if (i > 0 && !mac_append_char(dest, dest_size, &position, '.')) {
            dest[0] = '\0';
            return;
        }

        for (int bit = MAC_OCTET_BITS - 1; bit >= 0; bit--) {
            const char ch =
                (((unsigned int)addr->octets[i] >> (unsigned int)bit) & 1U) != 0U ? '1' : '0';

            if (!mac_append_char(dest, dest_size, &position, ch)) {
                dest[0] = '\0';
                return;
            }
        }
    }
}

void mac_format_octet_binary(uint8_t octet, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    for (int bit = MAC_OCTET_BITS - 1; bit >= 0; bit--) {
        const char ch = (((unsigned int)octet >> (unsigned int)bit) & 1U) != 0U ? '1' : '0';

        if (!mac_append_char(dest, dest_size, &position, ch)) {
            dest[0] = '\0';
            return;
        }
    }
}
