/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_IP_IPV6_SCREEN_H
#define VLSX_SCREENS_IP_IPV6_SCREEN_H

#include <stdbool.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/ipv6/ipv6_types.h"

typedef struct {
    input_field_t input;
    char buf[V6_INPUT_MAX];
    bool has_result;
    bool is_valid;
    calc_status_t status;
    ipv6_result_t result;
} ipv6_screen_t;

void ipv6_screen_init(ipv6_screen_t *screen);
void ipv6_screen_confirm(ipv6_screen_t *screen);
bool ipv6_screen_handle(ipv6_screen_t *screen, const ui_event_t *event);
const input_field_t *ipv6_screen_active_input(const ipv6_screen_t *screen);

#endif /* VLSX_SCREENS_IP_IPV6_SCREEN_H */
