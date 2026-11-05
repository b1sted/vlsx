/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_IP_IPV4_SCREEN_H
#define VLSX_SCREENS_IP_IPV4_SCREEN_H

#include <stdbool.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/ipv4/ipv4_types.h"

typedef struct {
    input_field_t input;
    char buf[V4_INPUT_MAX];
    bool has_result;
    bool is_valid;
    calc_status_t status;
    ipv4_result_t result;
} ipv4_screen_t;

void ipv4_screen_init(ipv4_screen_t *screen);
void ipv4_screen_confirm(ipv4_screen_t *screen);
bool ipv4_screen_handle(ipv4_screen_t *screen, const ui_event_t *event);
const input_field_t *ipv4_screen_active_input(const ipv4_screen_t *screen);

#endif /* VLSX_SCREENS_IP_IPV4_SCREEN_H */
