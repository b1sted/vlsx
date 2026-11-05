/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "termbox2.h"

#include "app.h"
#include "ui.h"

#include "screens/ip/screen_ip.h"

#include "screens/ip/v4.h"
#include "screens/ip/v6.h"
#include "screens/ip/vlsm.h"

typedef enum {
    IP_TAB_V4,
    IP_TAB_VLSM,
    IP_TAB_V6,
} ip_tab_t;

void render_ip_tabbar(const int start_x, const int tabbar_y,
                      const char *tabbar_menu[], int count) {
    int menu_x = start_x;
    for (int i = 0; i < count; i++) {
        int item_len = (int)strlen(tabbar_menu[i]);

        bool selected = (i == app.tabbar_cursor);
        uintattr_t fg = selected ? COLOR_SILVER | TB_BOLD : COLOR_BLACK;
        uintattr_t bg = selected ? COLOR_COBALT : COLOR_SILVER;

        int box_x = menu_x - 1;
        for (int row = box_x; row < box_x + item_len + 6; row++) {
            tb_set_cell(row, tabbar_y, ' ', fg, bg);
        }

        if (selected)
            tb_printf(menu_x, tabbar_y, fg, bg, "> %s <", tabbar_menu[i]);
        else
            tb_printf(menu_x, tabbar_y, fg, bg, "[ %s ]", tabbar_menu[i]);

        box_x += item_len + 6;
        menu_x += item_len + 8;
    }
}

void screen_ip_focus(void) {
    switch (app.tabbar_cursor) {
    case IP_TAB_V4:
        v4_focus_change();
        break;
    case IP_TAB_VLSM:
        vlsm_focus_change();
        break;
    case IP_TAB_V6:
        break;
    }
}

void screen_ip_confirm(void) {
    switch (app.tabbar_cursor) {
    case IP_TAB_V4:
        v4_input_confirm();
        break;
    case IP_TAB_VLSM:
        vlsm_input_confirm();
        break;
    case IP_TAB_V6:
        break;
    }
}

bool screen_ip_handle_event(struct tb_event *ev) {
    switch (app.tabbar_cursor) {
    case IP_TAB_V4:
        return false;
    case IP_TAB_VLSM:
        return vlsm_handle_event(ev);
    case IP_TAB_V6:
        return false;
    }

    return false;
}

void render_ip_content(const int start_x, const int start_y) {
    switch (app.tabbar_cursor) {
    case IP_TAB_V4:
        render_v4_content(start_x, start_y);
        break;
    case IP_TAB_VLSM:
        render_vlsm_content(start_x, start_y);
        break;
    case IP_TAB_V6:
        render_v6_content(start_x, start_y);
        break;
    }
}
