/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/metrics/goodput_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "common/limits.h"
#include "core/net/sla/sla_calc.h"
#include "screens/metrics/goodput_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define GOODPUT_LABEL_WIDTH 25

void goodput_view_render(const goodput_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const bool first_active = (screen->active_input == 0);

    ui_render_input_block(layout->content_x, layout->content_y, SLA_INPUT_MAX,
                          "BW:", &screen->fields[0], "100", sla_validate_number, first_active);
    ui_render_input_block(layout->content_x, layout->content_y + 1, SLA_INPUT_MAX,
                          "Overhead:", &screen->fields[1], "15", sla_validate_number,
                          !first_active);

    if (!screen->has_result) {
        return;
    }

    const int start_y = layout->content_y + 3;

    if (!screen->is_valid) {
        ui_render_input_error(layout->content_x, start_y, screen->status.message);
        return;
    }

    char nominal[MINI_BUFFER];
    char overhead[MINI_BUFFER];
    char goodput[MINI_BUFFER];

    snprintf(nominal, sizeof(nominal), "%.2f Mbps", screen->result.nominal_mbps);
    snprintf(overhead, sizeof(overhead), "%.2f Mbps", screen->result.overhead_mbps);
    snprintf(goodput, sizeof(goodput), "%.2f Mbps", screen->result.goodput_mbps);

    ui_render_section_title(layout->content_x, start_y, "[ GOODPUT CALCULATION ]");
    ui_render_label_value_pair(layout->content_x, start_y + 1, GOODPUT_LABEL_WIDTH,
                               "Nominal Bandwidth (T):", nominal);
    ui_render_label_value_pair(layout->content_x, start_y + 2, GOODPUT_LABEL_WIDTH,
                               "Overhead (O):", overhead);
    ui_render_label_value_pair(layout->content_x, start_y + 3, GOODPUT_LABEL_WIDTH,
                               "Effective Goodput (G):", goodput);
}
