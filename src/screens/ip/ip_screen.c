/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/ip/ip_screen.h"

#include <stdbool.h>
#include <stddef.h>

#include "screens/ip/ipv4_screen.h"
#include "screens/ip/vlsm_screen.h"

void ip_screen_init(ip_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->active_tab = IP_TAB_V4;
    ipv4_screen_init(&screen->ipv4);
    vlsm_screen_init(&screen->vlsm);
}

void ip_screen_set_tab(ip_screen_t *screen, size_t tab_index)
{
    if (screen == NULL || tab_index >= (size_t)IP_TAB_COUNT) {
        return;
    }

    screen->active_tab = (ip_tab_t)tab_index;
}

ip_tab_t ip_screen_active_tab(const ip_screen_t *screen)
{
    if (screen == NULL) {
        return IP_TAB_V4;
    }

    return screen->active_tab;
}

void ip_screen_focus(ip_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->vlsm.active_input = 0;
}

void ip_screen_confirm(ip_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    switch (screen->active_tab) {
    case IP_TAB_V4:
        ipv4_screen_confirm(&screen->ipv4);
        break;
    case IP_TAB_VLSM:
        vlsm_screen_confirm(&screen->vlsm);
        break;
    case IP_TAB_V6:
        break;
    case IP_TAB_COUNT:
        break;
    }
}

bool ip_screen_handle(ip_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (screen->active_tab) {
    case IP_TAB_V4:
        return ipv4_screen_handle(&screen->ipv4, event);
    case IP_TAB_VLSM:
        return vlsm_screen_handle(&screen->vlsm, event);
    case IP_TAB_V6:
        return false;
    case IP_TAB_COUNT:
        return false;
    }

    return false;
}
