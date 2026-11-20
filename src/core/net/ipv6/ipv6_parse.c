/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/ipv6/ipv6_parse.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_types.h"

#define IPV6_DEFAULT_PREFIX 64

static bool ipv6_char_is_hex(char ch);
static uint8_t ipv6_hex_value(char ch);
static bool ipv6_parse_hextet(const char *text, size_t length, uint16_t *value);
static bool ipv6_parse_ipv4_tail(const char *text, size_t length, uint16_t octet_groups[2]);
static bool ipv6_parse_component(const char *text, size_t length, bool allow_ipv4,
                                 uint16_t groups[IPV6_HEXTET_COUNT], size_t *count);
static bool ipv6_parse_prefix(const char *text, size_t length, uint8_t *prefix);
static bool ipv6_parse_address(const char *text, size_t length, ipv6_addr_t *addr);

static bool ipv6_char_is_hex(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static uint8_t ipv6_hex_value(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return (uint8_t)(ch - '0');
    }

    if (ch >= 'a' && ch <= 'f') {
        return (uint8_t)(ch - 'a' + 10);
    }

    return (uint8_t)(ch - 'A' + 10);
}

static bool ipv6_parse_hextet(const char *text, size_t length, uint16_t *value)
{
    uint16_t result = 0;

    if (text == NULL || value == NULL || length == 0 || length > 4) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        if (!ipv6_char_is_hex(text[i])) {
            return false;
        }

        result = (uint16_t)((result << 4) | (uint16_t)ipv6_hex_value(text[i]));
    }

    *value = result;
    return true;
}

static bool ipv6_parse_ipv4_tail(const char *text, size_t length, uint16_t octet_groups[2])
{
    uint16_t octets[4] = {0, 0, 0, 0};
    size_t cursor = 0;

    if (text == NULL || octet_groups == NULL) {
        return false;
    }

    for (size_t i = 0; i < 4; i++) {
        size_t digits = 0;
        uint16_t value = 0;

        while (cursor < length && text[cursor] != '.') {
            if (text[cursor] < '0' || text[cursor] > '9') {
                return false;
            }

            value = (uint16_t)(value * 10 + (uint16_t)(text[cursor] - '0'));
            digits++;
            cursor++;

            if (digits > 3 || value > 255) {
                return false;
            }
        }

        if (digits == 0) {
            return false;
        }

        octets[i] = value;

        if (i < 3) {
            if (cursor >= length || text[cursor] != '.') {
                return false;
            }

            cursor++;
        }
    }

    if (cursor != length) {
        return false;
    }

    octet_groups[0] = (uint16_t)((octets[0] << 8) | octets[1]);
    octet_groups[1] = (uint16_t)((octets[2] << 8) | octets[3]);

    return true;
}

static bool ipv6_parse_component(const char *text, size_t length, bool allow_ipv4,
                                 uint16_t groups[IPV6_HEXTET_COUNT], size_t *count)
{
    size_t cursor = 0;
    size_t parsed = 0;

    if (text == NULL || groups == NULL || count == NULL) {
        return false;
    }

    while (cursor < length) {
        size_t end = cursor;
        bool has_dot = false;

        while (end < length && text[end] != ':') {
            if (text[end] == '.') {
                has_dot = true;
            }

            end++;
        }

        const size_t segment_length = end - cursor;

        if (segment_length == 0) {
            return false;
        }

        if (has_dot) {
            uint16_t octet_groups[2] = {0, 0};

            if (!allow_ipv4 || end != length) {
                return false;
            }

            if (!ipv6_parse_ipv4_tail(text + cursor, segment_length, octet_groups)) {
                return false;
            }

            if (parsed + 2 > IPV6_HEXTET_COUNT) {
                return false;
            }

            groups[parsed] = octet_groups[0];
            groups[parsed + 1] = octet_groups[1];
            parsed += 2;
            cursor = length;
        } else {
            uint16_t value = 0;

            if (!ipv6_parse_hextet(text + cursor, segment_length, &value)) {
                return false;
            }

            if (parsed >= IPV6_HEXTET_COUNT) {
                return false;
            }

            groups[parsed] = value;
            parsed++;

            if (end == length) {
                cursor = length;
            } else {
                cursor = end + 1;

                if (cursor == length) {
                    return false;
                }
            }
        }
    }

    *count = parsed;
    return true;
}

static bool ipv6_parse_prefix(const char *text, size_t length, uint8_t *prefix)
{
    uint16_t value = 0;

    if (text == NULL || prefix == NULL || length == 0 || length > 3) {
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') {
            return false;
        }

        value = (uint16_t)(value * 10 + (uint16_t)(text[i] - '0'));
    }

    if (value > 128) {
        return false;
    }

    *prefix = (uint8_t)value;
    return true;
}

static bool ipv6_parse_address(const char *text, size_t length, ipv6_addr_t *addr)
{
    uint16_t groups[IPV6_HEXTET_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
    const char *double_colon = NULL;

    if (text == NULL || addr == NULL || length == 0) {
        return false;
    }

    for (size_t i = 0; i + 1 < length; i++) {
        if (text[i] == ':' && text[i + 1] == ':') {
            double_colon = text + i;
            break;
        }
    }

    memset(addr, 0, sizeof(*addr));

    if (double_colon == NULL) {
        size_t count = 0;

        if (!ipv6_parse_component(text, length, true, groups, &count) ||
            count != IPV6_HEXTET_COUNT) {
            return false;
        }

        memcpy(addr->hextets, groups, sizeof(groups));
        return true;
    }

    const size_t left_length = (size_t)(double_colon - text);
    const size_t right_offset = left_length + 2;
    const size_t right_length = length - right_offset;

    for (size_t i = right_offset; i + 1 < length; i++) {
        if (text[i] == ':' && text[i + 1] == ':') {
            return false;
        }
    }

    uint16_t left_groups[IPV6_HEXTET_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t right_groups[IPV6_HEXTET_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
    size_t left_count = 0;
    size_t right_count = 0;

    if (!ipv6_parse_component(text, left_length, false, left_groups, &left_count) ||
        !ipv6_parse_component(text + right_offset, right_length, true, right_groups,
                              &right_count)) {
        return false;
    }

    if (left_count + right_count > IPV6_HEXTET_COUNT - 1) {
        return false;
    }

    for (size_t i = 0; i < left_count; i++) {
        addr->hextets[i] = left_groups[i];
    }

    const size_t right_start = IPV6_HEXTET_COUNT - right_count;

    for (size_t i = 0; i < right_count; i++) {
        addr->hextets[right_start + i] = right_groups[i];
    }

    return true;
}

bool ipv6_parse_cidr(const char *input, ipv6_addr_t *base, uint8_t *prefix, calc_status_t *status)
{
    const char *slash = NULL;
    size_t address_length = 0;

    if (base == NULL || prefix == NULL || status == NULL) {
        return false;
    }

    memset(base, 0, sizeof(*base));
    *prefix = IPV6_DEFAULT_PREFIX;
    calc_status_clear(status);

    if (input == NULL || input[0] == '\0') {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid input.\nExpected format: IPv6-address[/prefix]");
        return false;
    }

    slash = strchr(input, '/');

    if (slash != NULL && strchr(slash + 1, '/') != NULL) {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid input.\nOnly one prefix is allowed");
        return false;
    }

    address_length = (slash != NULL) ? (size_t)(slash - input) : strlen(input);

    if (!ipv6_parse_address(input, address_length, base)) {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid input.\nExpected format: IPv6-address[/prefix]");
        return false;
    }

    if (slash != NULL) {
        const size_t prefix_length = strlen(slash + 1);

        if (!ipv6_parse_prefix(slash + 1, prefix_length, prefix)) {
            calc_status_fail(status, CALC_ERR_RANGE,
                             "Invalid input.\nPrefix length must be in range: [0, 128]");
            return false;
        }
    }

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
