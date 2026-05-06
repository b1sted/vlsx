/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "termbox2.h"

#include "app.h"
#include "ui.h"

#include "screens/ip/screen_ip.h"

#include "screens/ip/v4.h"
#include "screens/ip/v6.h"
#include "screens/ip/vlsm.h"

void render_ip_tabbar(const int start_x, const int tabbar_y,
                      const char *tabbar_menu[]) {
    int menu_x = start_x;
    for (int i = 0; tabbar_menu[i] != NULL; i++) {
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
    case 0:
        ipv4_focus_change();
        break;
    }

    app.input_cursor = 0;
    app.current_focus = FOCUS_INPUT;
}

void screen_ip_confirm(void) {
    switch (app.tabbar_cursor) {
    case 0:
        ipv4_input_confirm();
        break;
    }
}

void render_ip_content(const int start_x, const int start_y) {
    switch (app.tabbar_cursor) {
    case 0:
        render_ipv4_content(start_x, start_y);
        break;
    case 1:
        render_vlsm_content(start_x, start_y);
        break;
    case 2:
        render_ipv6_content(start_x, start_y);
        break;
    }
}
