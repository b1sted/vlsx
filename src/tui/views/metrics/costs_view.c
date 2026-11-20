/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/metrics/costs_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "common/limits.h"
#include "core/net/sla/sla_calc.h"
#include "screens/metrics/costs_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define COSTS_LABEL_WIDTH 15

void costs_view_render(const costs_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const bool first_active = (screen->active_input == 0);

    ui_render_input_block(layout->content_x, layout->content_y, SLA_COST_INPUT_MAX, "CapEx:", 1,
                          &screen->fields[0], "5000, 200, 1000", sla_validate_cost_list,
                          first_active);
    ui_render_input_block(layout->content_x, layout->content_y + 1, SLA_COST_INPUT_MAX, "OpEx:", 1,
                          &screen->fields[1], "100, 50, 20", sla_validate_cost_list, !first_active);

    if (!screen->has_result) {
        return;
    }

    const int start_y = layout->content_y + 3;

    if (!screen->is_valid) {
        ui_render_input_error(layout->content_x, start_y, screen->status.message);
        return;
    }

    char capex[MINI_BUFFER];
    char opex[MINI_BUFFER];
    char total[MINI_BUFFER];

    snprintf(capex, sizeof(capex), "$ %.2f", screen->result.capex);
    snprintf(opex, sizeof(opex), "$ %.2f", screen->result.opex);
    snprintf(total, sizeof(total), "$ %.2f", screen->result.total);

    ui_render_section_title(layout->content_x, start_y, "[ NETWORK COSTS ]");
    ui_render_label_value_pair(layout->content_x, start_y + 1, COSTS_LABEL_WIDTH,
                               "Total CapEx:", capex);
    ui_render_label_value_pair(layout->content_x, start_y + 2, COSTS_LABEL_WIDTH,
                               "Total OpEx:", opex);
    ui_render_label_value_pair(layout->content_x, start_y + 3, COSTS_LABEL_WIDTH,
                               "Total Cost:", total);
}
