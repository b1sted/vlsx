/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/l2/mac_screen.h"

#include <stdbool.h>
#include <stddef.h>

#include "screens/l2/mac_converter_screen.h"
#include "screens/l2/mac_multicast_screen.h"

void mac_screen_init(mac_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->active_tab = MAC_TAB_CONVERTER;
    mac_converter_screen_init(&screen->converter);
    mac_multicast_screen_init(&screen->multicast);
}

void mac_screen_set_tab(mac_screen_t *screen, size_t tab_index)
{
    if (screen == NULL || tab_index >= (size_t)MAC_TAB_COUNT) {
        return;
    }

    screen->active_tab = (mac_tab_t)tab_index;
}

mac_tab_t mac_screen_active_tab(const mac_screen_t *screen)
{
    if (screen == NULL) {
        return MAC_TAB_CONVERTER;
    }

    return screen->active_tab;
}

void mac_screen_focus(mac_screen_t *screen)
{
    (void)screen;
}

void mac_screen_confirm(mac_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    switch (screen->active_tab) {
    case MAC_TAB_CONVERTER:
        mac_converter_screen_confirm(&screen->converter);
        break;
    case MAC_TAB_MULTICAST:
        mac_multicast_screen_confirm(&screen->multicast);
        break;
    case MAC_TAB_COUNT:
        break;
    }
}

bool mac_screen_handle(mac_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (screen->active_tab) {
    case MAC_TAB_CONVERTER:
        return mac_converter_screen_handle(&screen->converter, event);
    case MAC_TAB_MULTICAST:
        return mac_multicast_screen_handle(&screen->multicast, event);
    case MAC_TAB_COUNT:
        return false;
    }

    return false;
}
