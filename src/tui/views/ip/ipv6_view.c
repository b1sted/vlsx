/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/ip/ipv6_view.h"

#include <stdbool.h>
#include <stdio.h>

#include "common/limits.h"
#include "core/net/ipv6/ipv6_class.h"
#include "core/net/ipv6/ipv6_types.h"
#include "core/net/ipv6/ipv6_validate.h"
#include "screens/ip/ipv6_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define IPV6_LABEL_WIDTH 15
#define IPV6_INPUT_WIDTH (V6_INPUT_MAX - 1)
#define IPV6_INPUT_LABEL_WIDTH 4
#define IPV6_STATUS_MAX_WIDTH 16

static void ipv6_view_render_information(const ipv6_screen_t *screen, const layout_t *layout,
                                         int y_offset);

static void ipv6_view_render_information(const ipv6_screen_t *screen, const layout_t *layout,
                                         int y_offset)
{
    const int start_x = layout->content_x;
    const int start_y = layout->content_y + y_offset;

    if (!screen->is_valid) {
        ui_render_input_error(start_x, start_y + 1, screen->status.message);
        return;
    }

    char prefix[TINY_BUFFER];

    snprintf(prefix, sizeof(prefix), "/%u", (unsigned int)screen->result.prefix);

    ui_render_section_title(start_x, start_y + 2, "[ FORMATTING & COMPRESSION ]");
    ui_render_label_value_pair(start_x, start_y + 3, IPV6_LABEL_WIDTH,
                               "Original:", screen->result.original);
    ui_render_label_value_pair(start_x, start_y + 4, IPV6_LABEL_WIDTH,
                               "No L-Zeros:", screen->result.no_leading_zeros);
    ui_render_label_value_pair(start_x, start_y + 5, IPV6_LABEL_WIDTH,
                               "Compressed:", screen->result.compressed);
    ui_render_label_value_pair(start_x, start_y + 6, IPV6_LABEL_WIDTH,
                               "Expanded:", screen->result.expanded);

    const char *interface_id =
        (screen->result.prefix == 128) ? "N/A (/128)" : screen->result.interface_id_text;

    ui_render_section_title(start_x, start_y + 8, "[ NETWORK INFORMATION ]");
    ui_render_label_value_pair(start_x, start_y + 9, IPV6_LABEL_WIDTH, "Prefix Length:", prefix);
    ui_render_label_value_pair(start_x, start_y + 10, IPV6_LABEL_WIDTH,
                               "Network Prefix:", screen->result.network_text);
    ui_render_label_value_pair(start_x, start_y + 11, IPV6_LABEL_WIDTH,
                               "Interface ID:", interface_id);
    ui_render_label_value_pair(start_x, start_y + 12, IPV6_LABEL_WIDTH,
                               "Address Type:", ipv6_type_name(screen->result.type));
}

void ipv6_view_render(const ipv6_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const int status_x = layout->content_x + IPV6_INPUT_LABEL_WIDTH + IPV6_INPUT_WIDTH;
    const int content_right = layout->content_x + layout->content_w;
    const bool status_inline = (status_x + IPV6_STATUS_MAX_WIDTH <= content_right);

    ui_render_input_block(layout->content_x, layout->content_y, IPV6_INPUT_WIDTH, "IP:", 1,
                          &screen->input, "fe80::a00:27:fea8:3ebd/64",
                          status_inline ? ipv6_validate : NULL, true);

    if (!status_inline && (!screen->has_result || screen->is_valid)) {
        ui_render_input_status(layout->content_x, layout->content_y + 1, &screen->input,
                               ipv6_validate);
    }

    if (!screen->has_result) {
        return;
    }

    if (!screen->is_valid) {
        ipv6_view_render_information(screen, layout, 0);
        return;
    }

    ipv6_view_render_information(screen, layout, status_inline ? 0 : 1);
}
