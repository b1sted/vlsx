/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_IP_IP_SCREEN_H
#define VLSX_SCREENS_IP_IP_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/ui_event.h"
#include "screens/ip/ipv4_screen.h"
#include "screens/ip/ipv6_screen.h"
#include "screens/ip/vlsm_screen.h"

typedef enum {
    IP_TAB_V4,
    IP_TAB_VLSM,
    IP_TAB_V6,
    IP_TAB_COUNT,
} ip_tab_t;

typedef struct {
    ip_tab_t active_tab;
    ipv4_screen_t ipv4;
    ipv6_screen_t ipv6;
    vlsm_screen_t vlsm;
} ip_screen_t;

void ip_screen_init(ip_screen_t *screen);
void ip_screen_set_tab(ip_screen_t *screen, size_t tab_index);
ip_tab_t ip_screen_active_tab(const ip_screen_t *screen);
void ip_screen_focus(ip_screen_t *screen);
void ip_screen_confirm(ip_screen_t *screen);
bool ip_screen_handle(ip_screen_t *screen, const ui_event_t *event);

#endif /* VLSX_SCREENS_IP_IP_SCREEN_H */
