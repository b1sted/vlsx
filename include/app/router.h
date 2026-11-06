/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_APP_ROUTER_H
#define VLSX_APP_ROUTER_H

#include "app/app.h"

void app_router_change_screen(app_state_t *state);
void app_router_change_subscreen(app_state_t *state);
void app_router_sync_tab(app_state_t *state);
void app_router_sync_mac_tab(app_state_t *state);

#endif /* VLSX_APP_ROUTER_H */
