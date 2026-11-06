/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "app/router.h"

#include <stddef.h>

#include "app/app.h"
#include "screens/ip/ip_screen.h"
#include "screens/l2/mac_screen.h"
#include "screens/metrics/metrics_screen.h"

void app_router_sync_tab(app_state_t *state)
{
    if (state == NULL) {
        return;
    }

    if (state->tabbar_cursor < 0 || state->tabbar_cursor >= (int)IP_TAB_COUNT) {
        state->tabbar_cursor = 0;
    }

    ip_screen_set_tab(&state->ip_screen, (size_t)state->tabbar_cursor);
}

void app_router_sync_mac_tab(app_state_t *state)
{
    if (state == NULL) {
        return;
    }

    if (state->mac_tabbar_cursor < 0 || state->mac_tabbar_cursor >= (int)MAC_TAB_COUNT) {
        state->mac_tabbar_cursor = 0;
    }

    mac_screen_set_tab(&state->mac_screen, (size_t)state->mac_tabbar_cursor);
}

void app_router_sync_metrics_tab(app_state_t *state)
{
    if (state == NULL) {
        return;
    }

    if (state->metrics_tabbar_cursor < 0 ||
        state->metrics_tabbar_cursor >= (int)METRICS_TAB_COUNT) {
        state->metrics_tabbar_cursor = 0;
    }

    metrics_screen_set_tab(&state->metrics_screen, (size_t)state->metrics_tabbar_cursor);
}

void app_router_change_screen(app_state_t *state)
{
    if (state == NULL) {
        return;
    }

    state->previous_screen = state->current_screen;
    state->previous_focus = state->current_focus;

    if (state->sidebar_cursor == 0) {
        state->current_screen = APP_SCR_IP;
        app_router_sync_tab(state);
        state->current_focus = FOCUS_TABBAR;
        return;
    }

    if (state->sidebar_cursor == 1) {
        state->current_screen = APP_SCR_MAC;
        app_router_sync_mac_tab(state);
        state->current_focus = FOCUS_TABBAR;
        return;
    }

    if (state->sidebar_cursor == 2) {
        state->current_screen = APP_SCR_METRICS;
        app_router_sync_metrics_tab(state);
        state->current_focus = FOCUS_TABBAR;
    }
}

void app_router_change_subscreen(app_state_t *state)
{
    if (state == NULL) {
        return;
    }

    switch (state->current_screen) {
    case APP_SCR_MAIN:
        break;
    case APP_SCR_IP:
        app_router_sync_tab(state);
        ip_screen_focus(&state->ip_screen);
        break;
    case APP_SCR_MAC:
        app_router_sync_mac_tab(state);
        mac_screen_focus(&state->mac_screen);
        break;
    case APP_SCR_METRICS:
        app_router_sync_metrics_tab(state);
        metrics_screen_focus(&state->metrics_screen);
        break;
    }
}
