/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_IP_VLSM_SCREEN_H
#define VLSX_SCREENS_IP_VLSM_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/vlsm/vlsm_types.h"

typedef struct {
    input_field_t fields[2];
    char cidr_buf[V4_INPUT_MAX];
    char hosts_buf[MINI_BUFFER];
    size_t active_input;
    bool has_result;
    bool is_valid;
    calc_status_t status;
    vlsm_result_t result;
    unsigned generation;
} vlsm_screen_t;

void vlsm_screen_init(vlsm_screen_t *screen);
void vlsm_screen_confirm(vlsm_screen_t *screen);
bool vlsm_screen_handle(vlsm_screen_t *screen, const ui_event_t *event);
const input_field_t *vlsm_screen_active_input(const vlsm_screen_t *screen);

#endif /* VLSX_SCREENS_IP_VLSM_SCREEN_H */
