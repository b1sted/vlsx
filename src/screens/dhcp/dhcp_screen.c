/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/dhcp/dhcp_screen.h"

#include <stdbool.h>
#include <stddef.h>

#include "screens/dhcp/dhcpv4_screen.h"

void dhcp_screen_init(dhcp_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->active_tab = DHCP_TAB_V4;
    dhcpv4_screen_init(&screen->v4);
}

void dhcp_screen_set_tab(dhcp_screen_t *screen, size_t tab_index)
{
    if (screen == NULL || tab_index >= (size_t)DHCP_TAB_COUNT) {
        return;
    }

    screen->active_tab = (dhcp_tab_t)tab_index;
}

dhcp_tab_t dhcp_screen_active_tab(const dhcp_screen_t *screen)
{
    if (screen == NULL) {
        return DHCP_TAB_V4;
    }

    return screen->active_tab;
}

void dhcp_screen_focus(dhcp_screen_t *screen)
{
    (void)screen;
}

void dhcp_screen_confirm(dhcp_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    switch (screen->active_tab) {
    case DHCP_TAB_V4:
        dhcpv4_screen_confirm(&screen->v4);
        break;
    case DHCP_TAB_COUNT:
        break;
    }
}

bool dhcp_screen_handle(dhcp_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (screen->active_tab) {
    case DHCP_TAB_V4:
        return dhcpv4_screen_handle(&screen->v4, event);
    case DHCP_TAB_COUNT:
        return false;
    }

    return false;
}
