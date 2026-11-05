/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/ip/ipv4_view.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "common/limits.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "core/net/ipv4/ipv4_class.h"
#include "core/net/ipv4/ipv4_validate.h"
#include "screens/ip/ipv4_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define IPV4_INFO_SECTION_MAX 34
#define IPV4_RANGE_SECTION_MAX 28
#define IPV4_HORIZONTAL_PADDING 2
#define IPV4_RANGE_TITLE_WIDTH 13

static const char *ipv4_view_network_text(const ipv4_result_t *result, char *buffer,
                                          size_t buffer_size);
static const char *ipv4_view_broadcast_text(const ipv4_result_t *result, char *buffer,
                                            size_t buffer_size);
static void ipv4_view_render_information(const ipv4_screen_t *screen, const layout_t *layout);

static const char *ipv4_view_network_text(const ipv4_result_t *result, char *buffer,
                                          size_t buffer_size)
{
    if (result->prefix == 32) {
        return "N/A (RFC 3021)";
    }

    ipv4_format_addr(&result->network, buffer, buffer_size);
    return buffer;
}

static const char *ipv4_view_broadcast_text(const ipv4_result_t *result, char *buffer,
                                            size_t buffer_size)
{
    if (result->prefix == 32) {
        return "N/A";
    }

    if (result->prefix == 31) {
        return "N/A (RFC 3021)";
    }

    ipv4_format_addr(&result->broadcast, buffer, buffer_size);
    return buffer;
}

static void ipv4_view_render_information(const ipv4_screen_t *screen, const layout_t *layout)
{
    const int start_x = layout->content_x;
    const int start_y = layout->content_y;

    if (!screen->is_valid) {
        ui_render_input_error(start_x, start_y + 1, screen->status.message);
        return;
    }

    char address[V4_INPUT_MAX];
    char mask[V4_INPUT_MAX];
    char network[V4_INPUT_MAX];
    char broadcast[V4_INPUT_MAX];
    char first_host[V4_INPUT_MAX];
    char last_host[V4_INPUT_MAX];
    char host_count[TINY_BUFFER];

    ipv4_format_addr(&screen->result.base, address, sizeof(address));
    ipv4_format_addr(&screen->result.mask, mask, sizeof(mask));
    ipv4_format_addr(&screen->result.first_host, first_host, sizeof(first_host));
    ipv4_format_addr(&screen->result.last_host, last_host, sizeof(last_host));
    snprintf(host_count, sizeof(host_count), "%" PRIu32, screen->result.host_count);

    ui_render_section_title(start_x, start_y + 1, "[ NETWORK INFORMATION ]");
    ui_render_label_value_pair(start_x, start_y + 2, V4_INPUT_MAX, "IP Address:", address);
    ui_render_label_value_pair(start_x, start_y + 3, V4_INPUT_MAX, "Subnet Mask:", mask);
    ui_render_label_value_pair(start_x, start_y + 4, V4_INPUT_MAX, "Network Address:",
                               ipv4_view_network_text(&screen->result, network, sizeof(network)));
    ui_render_label_value_pair(
        start_x, start_y + 5, V4_INPUT_MAX, "Broadcast Address:",
        ipv4_view_broadcast_text(&screen->result, broadcast, sizeof(broadcast)));

    const int needed_x =
        start_x + IPV4_INFO_SECTION_MAX + IPV4_HORIZONTAL_PADDING + IPV4_RANGE_SECTION_MAX;
    const int window_x = layout->win_x + layout->win_w - 2;
    const bool enough_size = needed_x <= window_x;

    const int range_section_x =
        enough_size ? start_x + IPV4_INFO_SECTION_MAX + IPV4_HORIZONTAL_PADDING : start_x;
    const int range_section_y = enough_size ? start_y + 1 : start_y + 7;
    const int range_section_width = enough_size ? IPV4_RANGE_TITLE_WIDTH : V4_INPUT_MAX;

    ui_render_section_title(range_section_x, range_section_y, "[ RANGE OF NODES ]");
    ui_render_label_value_pair(range_section_x, range_section_y + 1, range_section_width,
                               "First Host:", first_host);
    ui_render_label_value_pair(range_section_x, range_section_y + 2, range_section_width,
                               "Last Host:", last_host);
    ui_render_label_value_pair(range_section_x, range_section_y + 3, range_section_width,
                               "Total Hosts:", host_count);
    ui_render_label_value_pair(range_section_x, range_section_y + 4, range_section_width,
                               "Class:", ipv4_class_name(screen->result.class));

    const int binary_section_y = enough_size ? start_y + 7 : range_section_y + 6;

    ui_render_section_title(start_x, binary_section_y, "[ BINARY FORM ]");
    ui_render_bin_section(start_x, binary_section_y + 1, V4_INPUT_MAX,
                          "IP:", screen->result.base.octets);
    ui_render_bin_section(start_x, binary_section_y + 2, V4_INPUT_MAX,
                          "Mask:", screen->result.mask.octets);
}

void ipv4_view_render(const ipv4_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    ui_render_input_block(layout->content_x, layout->content_y, V4_INPUT_MAX, "IP:", 1,
                          &screen->input, "192.168.0.54/26", ipv4_validate, true);

    if (!screen->has_result) {
        return;
    }

    ipv4_view_render_information(screen, layout);
}
