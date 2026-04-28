/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "input.h"
#include "termbox2.h"

#include "app.h"
#include "events.h"

#include "screens/screen_ip.h"

static void cursor_up(int *cursor);
static void cursor_down(int *cursor, int *items_count);
static void change_screen(void);

static void handle_sidebar_event(struct tb_event *ev);
static void handle_tabbar_event(struct tb_event *ev);
static void handle_input_event(struct tb_event *ev);

static void cursor_up(int *cursor) {
    if (*cursor > 0)
        (*cursor)--;
}

static void cursor_down(int *cursor, int *items_count) {
    if (*cursor < *items_count - 1)
        (*cursor)++;
}

static void change_screen(void) {
    app.previous_screen = app.current_screen;
    app.previous_focus = app.current_focus;

    switch (app.sidebar_cursor) {
    case 0:
        app.current_screen = APP_SCR_IP;
        break;
    }

    app.current_focus = FOCUS_TABBAR;
    app.has_press_input = false;
}

static void handle_sidebar_event(struct tb_event *ev) {
    switch (ev->key) {
    case TB_KEY_ARROW_UP:
        cursor_up(&app.sidebar_cursor);
        break;

    case TB_KEY_ARROW_DOWN:
        cursor_down(&app.sidebar_cursor, &app.sidebar_items);
        break;

    case TB_KEY_ENTER:
    case TB_KEY_ARROW_RIGHT:
        change_screen();
        break;
    }
}

static void handle_tabbar_event(struct tb_event *ev) {
    switch (ev->key) {
    case TB_KEY_ARROW_RIGHT:
        cursor_down(&app.tabbar_cursor, &app.tabbar_items);
        break;

    case TB_KEY_ARROW_LEFT:
        cursor_up(&app.tabbar_cursor);
        break;

    case TB_KEY_ENTER:
        switch (app.current_screen) {
        case APP_SCR_MAIN:
            break;
        case APP_SCR_IP:
            screen_ip_focus();
            break;
        }
    }
}

static void handle_input_event(struct tb_event *ev) {
    switch (ev->key) {
    case TB_KEY_ENTER:
        app.has_press_input = true;

        switch (app.current_screen) {
        case APP_SCR_MAIN:
            break;
        case APP_SCR_IP:
            screen_ip_confirm();
            break;
        }

        break;

    case TB_KEY_BACKSPACE:
    case TB_KEY_BACKSPACE2:
        input_backspace(app.active_field);
        break;

    case TB_KEY_ARROW_RIGHT:
        if (app.active_field->cursor < app.active_field->len)
            app.active_field->cursor++;
        break;

    case TB_KEY_ARROW_LEFT:
        if (app.active_field->cursor > 0)
            app.active_field->cursor--;
        break;

    case TB_KEY_CTRL_U:
        input_field_clear(app.active_field);
        break;

    case TB_KEY_CTRL_A:
        app.active_field->cursor = 0;
        break;

    case TB_KEY_CTRL_E:
        app.active_field->cursor = app.active_field->len;
        break;

    case TB_KEY_ESC:
        app.current_focus = FOCUS_TABBAR;
        break;

    default:
        if (ev->ch && ev->ch <= 0x7F)
            input_insert(app.active_field, (char)ev->ch, app.active_accept);
    }
}

void handle_global_event(struct tb_event *ev) {
    switch (ev->key) {
    case 0:
        if (ev->ch == 'q')
            app.running = 0;
        break;

    case TB_KEY_ESC:
        if (app.current_screen == APP_SCR_MAIN) {
            app.running = false;
            break;
        }

        if (app.current_focus == FOCUS_INPUT)
            break;

        app.current_screen = app.previous_screen;
        app.current_focus = FOCUS_SIDEBAR;
        app.has_press_input = false;

        break;
    }

    switch (app.current_focus) {
    case FOCUS_SIDEBAR:
        handle_sidebar_event(ev);
        break;

    case FOCUS_TABBAR:
        handle_tabbar_event(ev);
        break;

    case FOCUS_INPUT:
        handle_input_event(ev);
        break;
    }
}
