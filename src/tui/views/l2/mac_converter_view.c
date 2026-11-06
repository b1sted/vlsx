/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/l2/mac_converter_view.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "common/limits.h"
#include "core/net/mac/mac_calc.h"
#include "core/net/mac/mac_parse.h"
#include "screens/l2/mac_converter_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define MAC_INPUT_WIDTH 32
#define MAC_INPUT_LABEL_WIDTH 4
#define MAC_LABEL_WIDTH 18
#define MAC_STATUS_MAX_WIDTH 16
#define MAC_DETAIL_WIDTH 10
#define MAC_BINARY_OCTETS_PER_LINE 3
#define MAC_BINARY_BITS_PER_LINE (MAC_BINARY_OCTETS_PER_LINE * MAC_OCTET_BITS)
#define MAC_BINARY_LINE_LENGTH (MAC_BINARY_BITS_PER_LINE + MAC_BINARY_OCTETS_PER_LINE - 1)
#define MAC_BINARY_LINE_TEXT_MAX (MAC_BINARY_LINE_LENGTH + 1)

static void mac_converter_view_render_information(const mac_converter_screen_t *screen,
                                                  const layout_t *layout);
static void mac_converter_view_render_binary(int x, int y, const char *binary);

static void mac_converter_view_render_binary(int x, int y, const char *binary)
{
    if (binary == NULL) {
        return;
    }

    if (strlen(binary) <= (size_t)MAC_BINARY_LINE_LENGTH) {
        ui_render_label_value_pair(x, y, MAC_LABEL_WIDTH, "Binary:", binary);
        return;
    }

    char first_line[MAC_BINARY_LINE_TEXT_MAX];
    char second_line[MAC_BINARY_LINE_TEXT_MAX];

    snprintf(first_line, sizeof(first_line), "%.*s", MAC_BINARY_LINE_LENGTH, binary);
    snprintf(second_line, sizeof(second_line), "%s", binary + MAC_BINARY_LINE_LENGTH + 1);

    ui_render_label_value_pair(x, y, MAC_LABEL_WIDTH, "Binary:", first_line);
    ui_render_label_value_pair(x, y + 1, MAC_LABEL_WIDTH, "", second_line);
}

static void mac_converter_view_render_information(const mac_converter_screen_t *screen,
                                                  const layout_t *layout)
{
    const int start_x = layout->content_x;
    const int start_y = layout->content_y;

    if (!screen->is_valid) {
        ui_render_input_error(start_x, start_y + 1, screen->status.message);
        return;
    }

    char bit_title[MEDIUM_BUFFER];
    char cast_value[MINI_BUFFER];
    char admin_value[MINI_BUFFER];
    const unsigned int cast_bit = (unsigned int)(screen->result.address.octets[0] & UINT8_C(0x01));
    const unsigned int admin_bit =
        (unsigned int)(screen->result.address.octets[0] & UINT8_C(0x02)) >> 1;

    snprintf(bit_title, sizeof(bit_title), "[ BIT ANALYSIS (1st Octet: %02X -> %s) ]",
             (unsigned int)screen->result.address.octets[0], screen->result.first_octet_binary);
    snprintf(cast_value, sizeof(cast_value), "%-*s(Bit 0 = %u)", MAC_DETAIL_WIDTH,
             mac_cast_name(screen->result.cast), cast_bit);
    snprintf(admin_value, sizeof(admin_value), "%-*s(Bit 1 = %u)", MAC_DETAIL_WIDTH,
             mac_admin_name(screen->result.admin), admin_bit);

    ui_render_section_title(start_x, start_y + 2, "[ MAC ADDRESS STRUCTURE ]");
    ui_render_label_value_pair(start_x, start_y + 3, MAC_LABEL_WIDTH,
                               "OUI (Vendor):", screen->result.oui);
    ui_render_label_value_pair(start_x, start_y + 4, MAC_LABEL_WIDTH,
                               "NIC (Device ID):", screen->result.nic);
    ui_render_section_title(start_x, start_y + 6, bit_title);
    ui_render_label_value_pair(start_x, start_y + 7, MAC_LABEL_WIDTH, "Cast Type:", cast_value);
    ui_render_label_value_pair(start_x, start_y + 8, MAC_LABEL_WIDTH, "Admin Type:", admin_value);
    ui_render_section_title(start_x, start_y + 10, "[ FORMATS ]");
    ui_render_label_value_pair(start_x, start_y + 11, MAC_LABEL_WIDTH,
                               "Linux/Windows:", screen->result.colon);
    ui_render_label_value_pair(start_x, start_y + 12, MAC_LABEL_WIDTH,
                               "Cisco:", screen->result.cisco);
    mac_converter_view_render_binary(start_x, start_y + 13, screen->result.binary);
}

void mac_converter_view_render(const mac_converter_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const int status_x = layout->content_x + MAC_INPUT_LABEL_WIDTH + 1 + MAC_INPUT_WIDTH;
    const int content_right = layout->content_x + layout->content_w;
    const bool status_inline = (status_x + MAC_STATUS_MAX_WIDTH <= content_right);

    ui_render_input_block(layout->content_x, layout->content_y, MAC_INPUT_WIDTH, "MAC:", 1,
                          &screen->input, "F4:46:37:55:24:86", status_inline ? mac_validate : NULL,
                          true);

    if (!status_inline && (!screen->has_result || screen->is_valid)) {
        ui_render_input_status(layout->content_x, layout->content_y + 1, &screen->input,
                               mac_validate);
    }

    if (!screen->has_result) {
        return;
    }

    mac_converter_view_render_information(screen, layout);
}
