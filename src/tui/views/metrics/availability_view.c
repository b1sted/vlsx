/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/metrics/availability_view.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "common/limits.h"
#include "core/net/sla/sla_calc.h"
#include "screens/metrics/availability_screen.h"
#include "tui/layout.h"
#include "tui/primitives.h"

#define AVAILABILITY_LABEL_WIDTH 31

static double availability_view_truncate(double value, double scale);

static double availability_view_truncate(double value, double scale)
{
    return (double)((int64_t)(value * scale)) / scale;
}

void availability_view_render(const availability_screen_t *screen, const layout_t *layout)
{
    if (screen == NULL || layout == NULL) {
        return;
    }

    const bool first_active = (screen->active_input == 0);

    ui_render_input_block(layout->content_x, layout->content_y, SLA_INPUT_MAX,
                          "Uptime:", &screen->fields[0], "8760", sla_validate_number, first_active);
    ui_render_input_block(layout->content_x, layout->content_y + 1, SLA_INPUT_MAX,
                          "Downtime:", &screen->fields[1], "2", sla_validate_number, !first_active);

    if (!screen->has_result) {
        return;
    }

    const int start_y = layout->content_y + 3;

    if (!screen->is_valid) {
        ui_render_input_error(layout->content_x, start_y, screen->status.message);
        return;
    }

    char availability[MINI_BUFFER];
    char downtime_9999[SMALL_BUFFER];
    char downtime_99999[SMALL_BUFFER];

    snprintf(availability, sizeof(availability), "%.3f %%", screen->result.availability_percent);
    snprintf(downtime_9999, sizeof(downtime_9999), "%.1f minutes / year",
             availability_view_truncate(screen->result.allowed_downtime_9999_minutes, 10.0));
    snprintf(downtime_99999, sizeof(downtime_99999), "%.2f minutes / year",
             availability_view_truncate(screen->result.allowed_downtime_99999_minutes, 100.0));

    ui_render_section_title(layout->content_x, start_y, "[ AVAILABILITY (SLA) ]");
    ui_render_label_value_pair(layout->content_x, start_y + 1, AVAILABILITY_LABEL_WIDTH,
                               "Availability:", availability);
    ui_render_label_value_pair(layout->content_x, start_y + 2, AVAILABILITY_LABEL_WIDTH,
                               "Downtime allowed for 99.99%:", downtime_9999);
    ui_render_label_value_pair(layout->content_x, start_y + 3, AVAILABILITY_LABEL_WIDTH,
                               "Downtime allowed for 99.999%:", downtime_99999);
}
