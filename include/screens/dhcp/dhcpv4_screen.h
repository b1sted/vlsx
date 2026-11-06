/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_DHCP_DHCPV4_SCREEN_H
#define VLSX_SCREENS_DHCP_DHCPV4_SCREEN_H

#include <stdbool.h>

#include "common/input_field.h"
#include "common/limits.h"
#include "common/result.h"
#include "common/ui_event.h"
#include "core/net/dhcp/dhcp_calc.h"

typedef struct {
    input_field_t input;
    char lease_buf[DHCP_LEASE_INPUT_MAX];
    bool has_result;
    bool is_valid;
    calc_status_t status;
    dhcp_timer_result_t result;
} dhcpv4_screen_t;

void dhcpv4_screen_init(dhcpv4_screen_t *screen);
void dhcpv4_screen_confirm(dhcpv4_screen_t *screen);
bool dhcpv4_screen_handle(dhcpv4_screen_t *screen, const ui_event_t *event);
const input_field_t *dhcpv4_screen_active_input(const dhcpv4_screen_t *screen);

#endif /* VLSX_SCREENS_DHCP_DHCPV4_SCREEN_H */
