/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/l2/mac_multicast_view.h"

#include <stdbool.h>

#include "core/net/mac/mac_multicast.h"
#include "screens/l2/mac_multicast_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"
#include "tui/theme.h"

#define MAC_MULTICAST_INPUT_LABEL_WIDTH 13
#define MAC_MULTICAST_INPUT_WIDTH 16
#define MAC_MULTICAST_LABEL_WIDTH 18
#define MAC_MULTICAST_STATUS_MAX_WIDTH 16

void mac_multicast_view_render(const mac_multicast_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const int start_x = layout->content_x;
    const int start_y = layout->content_y;
    const int status_x =
        start_x + MAC_MULTICAST_INPUT_LABEL_WIDTH + TUI_LABEL_VALUE_GAP + MAC_MULTICAST_INPUT_WIDTH;
    const int content_right = start_x + layout->content_w;
    const bool status_inline = (status_x + MAC_MULTICAST_STATUS_MAX_WIDTH <= content_right);

    ui_render_input_block(start_x, start_y, MAC_MULTICAST_INPUT_WIDTH,
                          MAC_MULTICAST_INPUT_LABEL_WIDTH, "Multicast IP:", &screen->input,
                          "239.1.1.15", status_inline ? mac_multicast_validate : NULL, true);

    if (!status_inline && (!screen->has_result || screen->is_valid)) {
        ui_render_input_status(start_x, start_y + 1, &screen->input, mac_multicast_validate);
    }

    if (!screen->has_result) {
        return;
    }

    if (!screen->is_valid) {
        ui_render_input_error(start_x, start_y + 2, screen->status.message);
        return;
    }

    ui_render_section_title(start_x, start_y + 2, "[ MULTICAST MAC GENERATION ]");
    ui_render_label_value_pair(start_x, start_y + 3, MAC_MULTICAST_LABEL_WIDTH,
                               "Generated MAC:", screen->result.mac_text);

    ui_render_section_title(start_x, start_y + 5, "[ GENERATION STEPS ]");
    ui_render_label_value_pair(start_x, start_y + 6, MAC_MULTICAST_LABEL_WIDTH,
                               "1. Default OUI:", screen->result.oui);
    ui_render_label_value_pair(start_x, start_y + 7, MAC_MULTICAST_LABEL_WIDTH,
                               "2. IP (Binary):", screen->result.ip_binary);
    ui_render_label_value_pair(start_x, start_y + 8, MAC_MULTICAST_LABEL_WIDTH,
                               "3. Extracted:", screen->result.extracted);
    ui_render_label_value_pair(start_x, start_y + 9, MAC_MULTICAST_LABEL_WIDTH,
                               "4. Merged MAC:", screen->result.mac_text);
}
