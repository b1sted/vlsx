/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "app.h"
#include "events.h"

#include "termbox2.h"

static void handle_sidebar_event(struct tb_event *ev);

void handle_global_event(struct tb_event *ev) {
    switch (ev->key) {
    case 0:
        if (ev->ch == 'q')
            app.running = 0;
        break;

    case TB_KEY_ESC:
        app.running = false;
        break;
    }

    switch (app.current_focus) {
    case FOCUS_SIDEBAR:
        handle_sidebar_event(ev);
        break;
    }
}

static void handle_sidebar_event(struct tb_event *ev) {
    switch (ev->key) {
    case TB_KEY_ARROW_UP:
        if (app.sidebar_cursor > 0)
            app.sidebar_cursor--;
        break;

    case TB_KEY_ARROW_DOWN:
        if (app.sidebar_cursor < app.menu_items - 1)
            app.sidebar_cursor++;
        break;
    }
}
