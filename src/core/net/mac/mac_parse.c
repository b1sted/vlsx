/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/mac/mac_parse.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "core/net/mac/mac_types.h"

#define MAC_COLON_TEXT_LENGTH 17
#define MAC_CISCO_TEXT_LENGTH 14
#define MAC_RAW_TEXT_LENGTH 12

static bool mac_char_is_hex(char ch);
static uint8_t mac_hex_value(char ch);
static bool mac_parse_byte(const char *text, uint8_t *value);
static bool mac_parse_pairs(const char *text, size_t length, char separator,
                            uint8_t octets[MAC_OCTET_COUNT]);
static bool mac_parse_cisco(const char *text, size_t length, uint8_t octets[MAC_OCTET_COUNT]);
static bool mac_parse_raw(const char *text, size_t length, uint8_t octets[MAC_OCTET_COUNT]);

static bool mac_char_is_hex(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static uint8_t mac_hex_value(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return (uint8_t)(ch - '0');
    }

    if (ch >= 'a' && ch <= 'f') {
        return (uint8_t)(ch - 'a' + 10);
    }

    return (uint8_t)(ch - 'A' + 10);
}

static bool mac_parse_byte(const char *text, uint8_t *value)
{
    if (text == NULL || value == NULL || !mac_char_is_hex(text[0]) || !mac_char_is_hex(text[1])) {
        return false;
    }

    *value = (uint8_t)((mac_hex_value(text[0]) << 4) | mac_hex_value(text[1]));
    return true;
}

static bool mac_parse_pairs(const char *text, size_t length, char separator,
                            uint8_t octets[MAC_OCTET_COUNT])
{
    if (text == NULL || octets == NULL || length != MAC_COLON_TEXT_LENGTH) {
        return false;
    }

    for (size_t i = 0; i < MAC_OCTET_COUNT; i++) {
        const size_t offset = i * 3;

        if (!mac_parse_byte(text + offset, &octets[i])) {
            return false;
        }

        if (i + 1 < MAC_OCTET_COUNT && text[offset + 2] != separator) {
            return false;
        }
    }

    return true;
}

static bool mac_parse_cisco(const char *text, size_t length, uint8_t octets[MAC_OCTET_COUNT])
{
    if (text == NULL || octets == NULL || length != MAC_CISCO_TEXT_LENGTH) {
        return false;
    }

    for (size_t i = 0; i < MAC_OCTET_COUNT / 2; i++) {
        const size_t offset = i * 5;

        if (!mac_char_is_hex(text[offset]) || !mac_char_is_hex(text[offset + 1]) ||
            !mac_char_is_hex(text[offset + 2]) || !mac_char_is_hex(text[offset + 3])) {
            return false;
        }

        octets[i * 2] =
            (uint8_t)((mac_hex_value(text[offset]) << 4) | mac_hex_value(text[offset + 1]));
        octets[(i * 2) + 1] =
            (uint8_t)((mac_hex_value(text[offset + 2]) << 4) | mac_hex_value(text[offset + 3]));

        if (i + 1 < MAC_OCTET_COUNT / 2 && text[offset + 4] != '.') {
            return false;
        }
    }

    return true;
}

static bool mac_parse_raw(const char *text, size_t length, uint8_t octets[MAC_OCTET_COUNT])
{
    if (text == NULL || octets == NULL || length != MAC_RAW_TEXT_LENGTH) {
        return false;
    }

    for (size_t i = 0; i < MAC_OCTET_COUNT; i++) {
        if (!mac_parse_byte(text + (i * 2), &octets[i])) {
            return false;
        }
    }

    return true;
}

bool mac_parse(const char *input, mac_addr_t *out, calc_status_t *status)
{
    mac_addr_t parsed = {0};
    size_t length = 0;
    bool ok = false;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (input == NULL || input[0] == '\0') {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid MAC address.\nExpected format: XX:XX:XX:XX:XX:XX");
        return false;
    }

    length = strlen(input);

    ok = mac_parse_pairs(input, length, ':', parsed.octets) ||
         mac_parse_pairs(input, length, '-', parsed.octets) ||
         mac_parse_cisco(input, length, parsed.octets) ||
         mac_parse_raw(input, length, parsed.octets);

    if (!ok) {
        calc_status_fail(status, CALC_ERR_SYNTAX,
                         "Invalid MAC address.\nExpected format: XX:XX:XX:XX:XX:XX");
        return false;
    }

    *out = parsed;
    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}

bool mac_validate(const char *input)
{
    mac_addr_t address = {0};
    calc_status_t status = {0};

    return mac_parse(input, &address, &status);
}

bool mac_accept(char ch)
{
    return mac_char_is_hex(ch) || ch == ':' || ch == '-' || ch == '.';
}
