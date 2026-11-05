/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "app/events.h"

#include <stdbool.h>
#include <stdint.h>

#include "app/app.h"
#include "app/menus.h"
#include "app/router.h"
#include "common/ui_event.h"
#include "screens/ip/ip_screen.h"

static void cursor_up(int *cursor);
static void cursor_down(int *cursor, int item_count);
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
    switch (event->kind) {
    case UI_EVENT_KEY_RIGHT:
        cursor_down(&app.tabbar_cursor, APP_MENU_IP_TAB_COUNT);
        app_router_sync_tab(&app);
        break;
    case UI_EVENT_KEY_LEFT:
        cursor_up(&app.tabbar_cursor);
        app_router_sync_tab(&app);
        break;
    case UI_EVENT_KEY_ENTER:
        app.current_focus = FOCUS_INPUT;
        app_router_change_subscreen(&app);
        break;
    default:
        break;
    }
}

static void handle_input_event(const ui_event_t *event)
{
    if (app.current_screen == APP_SCR_IP) {
        ip_screen_handle(&app.ip_screen, event);
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
