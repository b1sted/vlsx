/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_METRICS_COSTS_SCREEN_H
#define VLSX_SCREENS_METRICS_COSTS_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/sla/sla_calc.h"

typedef struct {
    input_field_t fields[2];
    char capex_buf[SLA_COST_INPUT_MAX];
    char opex_buf[SLA_COST_INPUT_MAX];
    size_t active_input;
    bool has_result;
    bool is_valid;
    calc_status_t status;
    sla_costs_result_t result;
} costs_screen_t;

void costs_screen_init(costs_screen_t *screen);
void costs_screen_confirm(costs_screen_t *screen);
bool costs_screen_handle(costs_screen_t *screen, const ui_event_t *event);
const input_field_t *costs_screen_active_input(const costs_screen_t *screen);

#endif /* VLSX_SCREENS_METRICS_COSTS_SCREEN_H */
