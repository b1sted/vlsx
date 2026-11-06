/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_METRICS_GOODPUT_SCREEN_H
#define VLSX_SCREENS_METRICS_GOODPUT_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/sla/sla_calc.h"

typedef struct {
    input_field_t fields[2];
    char bandwidth_buf[SLA_INPUT_MAX];
    char overhead_buf[SLA_INPUT_MAX];
    size_t active_input;
    bool has_result;
    bool is_valid;
    calc_status_t status;
    sla_goodput_result_t result;
} goodput_screen_t;

void goodput_screen_init(goodput_screen_t *screen);
void goodput_screen_confirm(goodput_screen_t *screen);
bool goodput_screen_handle(goodput_screen_t *screen, const ui_event_t *event);
const input_field_t *goodput_screen_active_input(const goodput_screen_t *screen);

#endif /* VLSX_SCREENS_METRICS_GOODPUT_SCREEN_H */
