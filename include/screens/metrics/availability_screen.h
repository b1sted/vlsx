/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_METRICS_AVAILABILITY_SCREEN_H
#define VLSX_SCREENS_METRICS_AVAILABILITY_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/sla/sla_calc.h"

typedef struct {
    input_field_t fields[2];
    char uptime_buf[SLA_INPUT_MAX];
    char downtime_buf[SLA_INPUT_MAX];
    size_t active_input;
    bool has_result;
    bool is_valid;
    calc_status_t status;
    sla_availability_result_t result;
} availability_screen_t;

void availability_screen_init(availability_screen_t *screen);
void availability_screen_confirm(availability_screen_t *screen);
bool availability_screen_handle(availability_screen_t *screen, const ui_event_t *event);
const input_field_t *availability_screen_active_input(const availability_screen_t *screen);

#endif /* VLSX_SCREENS_METRICS_AVAILABILITY_SCREEN_H */
