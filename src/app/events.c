/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "app/events.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app/app.h"
#include "app/menus.h"
#include "app/router.h"
#include "common/ui_event.h"
#include "screens/dhcp/dhcp_screen.h"
#include "screens/ip/ip_screen.h"
#include "screens/l2/mac_screen.h"
#include "screens/metrics/metrics_screen.h"

static void cursor_up(int *cursor);
static void cursor_down(int *cursor, int item_count);
static int *active_tab_cursor(void);
static int active_tab_count(void);
static void sync_active_tab(void);
static void cycle_active_tab(void);
static void handle_sidebar_event(const ui_event_t *event);
static void handle_tabbar_event(const ui_event_t *event);
static void handle_input_event(const ui_event_t *event);

static void cursor_up(int *cursor)
{
    if (cursor != NULL && *cursor > 0) {
        (*cursor)--;
    }
}

static void cursor_down(int *cursor, int item_count)
{
    if (cursor != NULL && item_count > 0 && *cursor < item_count - 1) {
        (*cursor)++;
    }
}

static int *active_tab_cursor(void)
{
    if (app.current_screen == APP_SCR_IP) {
        return &app.tabbar_cursor;
    }

    if (app.current_screen == APP_SCR_MAC) {
        return &app.mac_tabbar_cursor;
    }

    if (app.current_screen == APP_SCR_METRICS) {
        return &app.metrics_tabbar_cursor;
    }

    if (app.current_screen == APP_SCR_DHCP) {
        return &app.dhcp_tabbar_cursor;
    }

    return NULL;
}

static int active_tab_count(void)
{
    if (app.current_screen == APP_SCR_IP) {
        return (int)IP_TAB_COUNT;
    }

    if (app.current_screen == APP_SCR_MAC) {
        return (int)MAC_TAB_COUNT;
    }

    if (app.current_screen == APP_SCR_METRICS) {
        return (int)METRICS_TAB_COUNT;
    }

    if (app.current_screen == APP_SCR_DHCP) {
        return (int)DHCP_TAB_COUNT;
    }

    return 0;
}

static void sync_active_tab(void)
{
    if (app.current_screen == APP_SCR_IP) {
        app_router_sync_tab(&app);
    } else if (app.current_screen == APP_SCR_MAC) {
        app_router_sync_mac_tab(&app);
    } else if (app.current_screen == APP_SCR_METRICS) {
        app_router_sync_metrics_tab(&app);
    } else if (app.current_screen == APP_SCR_DHCP) {
        app_router_sync_dhcp_tab(&app);
    }
}

static void cycle_active_tab(void)
{
    int *cursor = active_tab_cursor();
    const int tab_count = active_tab_count();

    if (cursor == NULL || tab_count <= 0) {
        return;
    }

    *cursor = (*cursor + 1) % tab_count;
    sync_active_tab();
}

static void handle_sidebar_event(const ui_event_t *event)
{
    switch (event->kind) {
    case UI_EVENT_KEY_UP:
        cursor_up(&app.sidebar_cursor);
        break;
    case UI_EVENT_KEY_DOWN:
        cursor_down(&app.sidebar_cursor, APP_MENU_MAIN_COUNT);
        break;
    case UI_EVENT_KEY_ENTER:
    case UI_EVENT_KEY_RIGHT:
        app_router_change_screen(&app);
        break;
    default:
        break;
    }
}

static void handle_tabbar_event(const ui_event_t *event)
{
    int *cursor = active_tab_cursor();
    const int tab_count = active_tab_count();

    if (cursor == NULL || tab_count <= 0) {
        return;
    }

    switch (event->kind) {
    case UI_EVENT_KEY_RIGHT:
        cursor_down(cursor, tab_count);
        sync_active_tab();
        break;
    case UI_EVENT_KEY_LEFT:
        cursor_up(cursor);
        sync_active_tab();
        break;
    case UI_EVENT_KEY_ENTER:
        app.current_focus = FOCUS_INPUT;
        app_router_change_subscreen(&app);
        break;
    case UI_EVENT_KEY_CHAR:
        if (event->ch >= (uint32_t)'1' && event->ch <= (uint32_t)'9') {
            const int tab_index = (int)(event->ch - (uint32_t)'1');

            if (tab_index < tab_count) {
                *cursor = tab_index;
                sync_active_tab();
            }
        }
        break;
    default:
        break;
    }
}

static void handle_input_event(const ui_event_t *event)
{
    if (app.current_screen == APP_SCR_IP) {
        ip_screen_handle(&app.ip_screen, event);
    } else if (app.current_screen == APP_SCR_MAC) {
        mac_screen_handle(&app.mac_screen, event);
    } else if (app.current_screen == APP_SCR_METRICS) {
        metrics_screen_handle(&app.metrics_screen, event);
    } else if (app.current_screen == APP_SCR_DHCP) {
        dhcp_screen_handle(&app.dhcp_screen, event);
    }
}

void app_handle_event(const ui_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (event->kind == UI_EVENT_KEY_CHAR && event->ch == (uint32_t)'q') {
        app.running = false;
        return;
    }

    if (event->kind == UI_EVENT_KEY_TAB) {
        cycle_active_tab();
        return;
    }

    if (event->kind == UI_EVENT_KEY_ESC) {
        if (app.current_screen == APP_SCR_MAIN) {
            app.running = false;
            return;
        }

        if (app.current_focus == FOCUS_INPUT) {
            app.current_focus = FOCUS_TABBAR;
            return;
        }

        app.current_screen = app.previous_screen;
        app.current_focus = FOCUS_SIDEBAR;
        return;
    }

    switch (app.current_focus) {
    case FOCUS_SIDEBAR:
        handle_sidebar_event(event);
        break;
    case FOCUS_TABBAR:
        handle_tabbar_event(event);
        break;
    case FOCUS_INPUT:
        handle_input_event(event);
        break;
    }
}
