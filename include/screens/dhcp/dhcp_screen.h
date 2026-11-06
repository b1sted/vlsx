/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_DHCP_DHCP_SCREEN_H
#define VLSX_SCREENS_DHCP_DHCP_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/ui_event.h"
#include "screens/dhcp/dhcpv4_screen.h"

typedef enum {
    DHCP_TAB_V4,
    DHCP_TAB_COUNT,
} dhcp_tab_t;

typedef struct {
    dhcp_tab_t active_tab;
    dhcpv4_screen_t v4;
} dhcp_screen_t;

void dhcp_screen_init(dhcp_screen_t *screen);
void dhcp_screen_set_tab(dhcp_screen_t *screen, size_t tab_index);
dhcp_tab_t dhcp_screen_active_tab(const dhcp_screen_t *screen);
void dhcp_screen_focus(dhcp_screen_t *screen);
void dhcp_screen_confirm(dhcp_screen_t *screen);
bool dhcp_screen_handle(dhcp_screen_t *screen, const ui_event_t *event);

#endif /* VLSX_SCREENS_DHCP_DHCP_SCREEN_H */
