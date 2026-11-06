/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_L2_MAC_SCREEN_H
#define VLSX_SCREENS_L2_MAC_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/ui_event.h"
#include "screens/l2/mac_converter_screen.h"
#include "screens/l2/mac_multicast_screen.h"

typedef enum {
    MAC_TAB_CONVERTER,
    MAC_TAB_MULTICAST,
    MAC_TAB_COUNT,
} mac_tab_t;

typedef struct {
    mac_tab_t active_tab;
    mac_converter_screen_t converter;
    mac_multicast_screen_t multicast;
} mac_screen_t;

void mac_screen_init(mac_screen_t *screen);
void mac_screen_set_tab(mac_screen_t *screen, size_t tab_index);
mac_tab_t mac_screen_active_tab(const mac_screen_t *screen);
void mac_screen_focus(mac_screen_t *screen);
void mac_screen_confirm(mac_screen_t *screen);
bool mac_screen_handle(mac_screen_t *screen, const ui_event_t *event);

#endif /* VLSX_SCREENS_L2_MAC_SCREEN_H */
