/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/mac/mac_multicast.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common/result.h"
#include "core/net/mac/mac_types.h"

#define MAC_MULTICAST_IP_VERSION_BITS 28
#define MAC_MULTICAST_IP_CLASS_D UINT32_C(0x0E)
#define MAC_MULTICAST_TAIL_MASK UINT32_C(0x007FFFFF)
#define MAC_MULTICAST_IPV4_MAX_OCTET 255
#define MAC_MULTICAST_IPV4_OCTET_COUNT 4

static bool mac_multicast_parse_ipv4(const char *text, uint32_t *out);
static bool mac_multicast_append_char(char *dest, size_t dest_size, size_t *position, char ch);
static void mac_multicast_format_ipv4(uint32_t ip, char *dest, size_t dest_size);
static void mac_multicast_format_ip_binary(uint32_t ip, char *dest, size_t dest_size);
static void mac_multicast_format_extracted(uint32_t ip, char *dest, size_t dest_size);

static bool mac_multicast_parse_ipv4(const char *text, uint32_t *out)
{
    uint32_t value = 0;
    size_t position = 0;

    if (text == NULL || out == NULL) {
        return false;
    }

    for (size_t octet = 0; octet < (size_t)MAC_MULTICAST_IPV4_OCTET_COUNT; octet++) {
        uint32_t part = 0;
        size_t digits = 0;

        if (text[position] < '0' || text[position] > '9') {
            return false;
        }

        while (text[position] >= '0' && text[position] <= '9') {
            if (digits >= 3) {
                return false;
            }

            part = (part * UINT32_C(10)) + (uint32_t)(text[position] - '0');

            if (part > MAC_MULTICAST_IPV4_MAX_OCTET) {
                return false;
            }

            digits++;
            position++;
        }

        value = (value << 8) | part;

        if (octet + 1 < (size_t)MAC_MULTICAST_IPV4_OCTET_COUNT) {
            if (text[position] != '.') {
                return false;
            }

            position++;
        }
    }

    if (text[position] != '\0') {
        return false;
    }

    *out = value;
    return true;
}

static bool mac_multicast_append_char(char *dest, size_t dest_size, size_t *position, char ch)
{
    if (*position + 1 >= dest_size) {
        return false;
    }

    dest[*position] = ch;
    (*position)++;
    dest[*position] = '\0';

    return true;
}

static void mac_multicast_format_ipv4(uint32_t ip, char *dest, size_t dest_size)
{
    if (dest == NULL || dest_size == 0) {
        return;
    }

    snprintf(dest, dest_size, "%u.%u.%u.%u", (unsigned int)((ip >> 24) & UINT32_C(0xFF)),
             (unsigned int)((ip >> 16) & UINT32_C(0xFF)),
             (unsigned int)((ip >> 8) & UINT32_C(0xFF)), (unsigned int)(ip & UINT32_C(0xFF)));
}

static void mac_multicast_format_ip_binary(uint32_t ip, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    for (int bit = 31; bit >= 0; bit--) {
        const char ch = ((ip >> (unsigned int)bit) & UINT32_C(1)) != 0U ? '1' : '0';

        if (!mac_multicast_append_char(dest, dest_size, &position, ch)) {
            dest[0] = '\0';
            return;
        }

        if (bit != 0 && bit % 8 == 0) {
            if (!mac_multicast_append_char(dest, dest_size, &position, '.')) {
                dest[0] = '\0';
                return;
            }
        }
    }
}

static void mac_multicast_format_extracted(uint32_t ip, char *dest, size_t dest_size)
{
    size_t position = 0;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    dest[0] = '\0';

    for (int bit = 31; bit >= 0; bit--) {
        char ch = '.';

        if (bit < 23) {
            ch = ((ip >> (unsigned int)bit) & UINT32_C(1)) != 0U ? '1' : '0';
        }

        if (!mac_multicast_append_char(dest, dest_size, &position, ch)) {
            dest[0] = '\0';
            return;
        }

        if (bit != 0 && bit % 8 == 0) {
            if (!mac_multicast_append_char(dest, dest_size, &position, '.')) {
                dest[0] = '\0';
                return;
            }
        }
    }
}

bool mac_multicast_generate(const char *text, mac_multicast_result_t *out, calc_status_t *status)
{
    uint32_t ip = 0;
    uint32_t tail = 0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!mac_multicast_parse_ipv4(text, &ip)) {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid IPv4 address.\nExpected format: A.B.C.D");
        return false;
    }

    if ((ip >> MAC_MULTICAST_IP_VERSION_BITS) != MAC_MULTICAST_IP_CLASS_D) {
        calc_status_fail(status, CALC_ERR_RANGE, "Not a Multicast IP");
        return false;
    }

    tail = ip & MAC_MULTICAST_TAIL_MASK;

    out->ip = ip;
    out->address.octets[0] = UINT8_C(0x01);
    out->address.octets[1] = UINT8_C(0x00);
    out->address.octets[2] = UINT8_C(0x5E);
    out->address.octets[3] = (uint8_t)((tail >> 16) & UINT32_C(0xFF));
    out->address.octets[4] = (uint8_t)((tail >> 8) & UINT32_C(0xFF));
    out->address.octets[5] = (uint8_t)(tail & UINT32_C(0xFF));

    mac_multicast_format_ipv4(ip, out->ip_text, sizeof(out->ip_text));
    mac_multicast_format_ip_binary(ip, out->ip_binary, sizeof(out->ip_binary));
    mac_multicast_format_extracted(ip, out->extracted, sizeof(out->extracted));

    snprintf(out->oui, sizeof(out->oui), "%02x:%02x:%02x", (unsigned int)out->address.octets[0],
             (unsigned int)out->address.octets[1], (unsigned int)out->address.octets[2]);
    snprintf(out->mac_text, sizeof(out->mac_text), "%02x:%02x:%02x:%02x:%02x:%02x",
             (unsigned int)out->address.octets[0], (unsigned int)out->address.octets[1],
             (unsigned int)out->address.octets[2], (unsigned int)out->address.octets[3],
             (unsigned int)out->address.octets[4], (unsigned int)out->address.octets[5]);

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}

bool mac_multicast_validate(const char *text)
{
    uint32_t ip = 0;

    if (!mac_multicast_parse_ipv4(text, &ip)) {
        return false;
    }

    return (ip >> MAC_MULTICAST_IP_VERSION_BITS) == MAC_MULTICAST_IP_CLASS_D;
}

bool mac_multicast_accept(char ch)
{
    return (ch >= '0' && ch <= '9') || ch == '.';
}
