/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/views/l2/mac_tabbar.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "termbox2.h"

#include "app/app.h"
#include "app/menus.h"
#include "tui/layout.h"
#include "tui/theme.h"

void mac_tabbar_render(const app_state_t *state, const layout_t *layout)
{
    if (state == NULL || layout == NULL) {
        return;
    }

    int menu_x = layout->tabbar_x;

    for (size_t i = 0; i < (size_t)APP_MENU_MAC_TAB_COUNT; i++) {
        const char *item = app_menu_mac_tab_items[i];
        const int item_len = (int)strlen(item);
        const bool selected = ((int)i == state->mac_tabbar_cursor);
        const uintattr_t fg = selected ? TUI_COLOR_SILVER | TB_BOLD : TUI_COLOR_BLACK;
        const uintattr_t bg = selected ? TUI_COLOR_COBALT : TUI_COLOR_SILVER;

        const int box_x = menu_x - 1;

        for (int x = box_x; x < box_x + item_len + 6; x++) {
            tb_set_cell(x, layout->tabbar_y, ' ', fg, bg);
        }

        if (selected) {
            tb_printf(menu_x, layout->tabbar_y, fg, bg, "> %s <", item);
        } else {
            tb_printf(menu_x, layout->tabbar_y, fg, bg, "[ %s ]", item);
        }

        menu_x += item_len + 8;
    }
}
