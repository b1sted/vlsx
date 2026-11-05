/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/chrome.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "termbox2.h"

#include "app/app.h"
#include "app/menus.h"
#include "tui/layout.h"
#include "tui/theme.h"

static void chrome_draw_outline(const layout_t *layout, int win_right, int win_bottom);

static void chrome_draw_outline(const layout_t *layout, int win_right, int win_bottom)
{
    tb_set_cell(layout->win_x, layout->win_y, TUI_BOX_TL, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    tb_set_cell(win_right - 1, layout->win_y, TUI_BOX_TR, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    tb_set_cell(layout->win_x, win_bottom - 1, TUI_BOX_BL, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    tb_set_cell(win_right - 1, win_bottom - 1, TUI_BOX_BR, TUI_COLOR_BLACK, TUI_COLOR_SILVER);

    for (int y = layout->win_y + 1; y < win_bottom - 1; y++) {
        tb_set_cell(layout->win_x, y, TUI_BOX_V, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
        tb_set_cell(win_right - 1, y, TUI_BOX_V, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    }

    tb_set_cell(layout->win_x, layout->win_y + 2, TUI_BOX_VR, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    tb_set_cell(win_right - 1, layout->win_y + 2, TUI_BOX_VL, TUI_COLOR_BLACK, TUI_COLOR_SILVER);

    for (int x = layout->win_x + 1; x < win_right - 1; x++) {
        tb_set_cell(x, layout->win_y, TUI_BOX_H, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
        tb_set_cell(x, layout->win_y + 2, TUI_BOX_H, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
        tb_set_cell(x, win_bottom - 1, TUI_BOX_H, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    }
}

void chrome_draw_background(int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tb_set_cell(x, y, ' ', TUI_COLOR_COBALT, TUI_COLOR_COBALT);
        }
    }
}

void chrome_draw_window(const layout_t *layout)
{
    if (layout == NULL) {
        return;
    }

    const int win_right = layout->win_x + layout->win_w;
    const int win_bottom = layout->win_y + layout->win_h;

    for (int y = layout->win_y; y < win_bottom; y++) {
        for (int x = layout->win_x; x < win_right; x++) {
            tb_set_cell(x, y, ' ', TUI_COLOR_SILVER, TUI_COLOR_SILVER);
        }
    }

    for (int x = layout->win_x + 2; x < win_right + 2; x++) {
        tb_set_cell(x, win_bottom, ' ', TUI_COLOR_CHARCOAL, TUI_COLOR_CHARCOAL);
    }

    for (int y = layout->win_y + 1; y < win_bottom; y++) {
        tb_set_cell(win_right, y, ' ', TUI_COLOR_CHARCOAL, TUI_COLOR_CHARCOAL);
        tb_set_cell(win_right + 1, y, ' ', TUI_COLOR_CHARCOAL, TUI_COLOR_CHARCOAL);
    }

    chrome_draw_outline(layout, win_right, win_bottom);
}

void chrome_draw_titlebar(const layout_t *layout)
{
    if (layout == NULL) {
        return;
    }

    tb_print(layout->win_x + 1, layout->win_y + 1, TUI_COLOR_COBALT | TB_BOLD, TUI_COLOR_SILVER,
             "vlsx " APP_VERSION);
}

void chrome_draw_main_menu(const app_state_t *state, const layout_t *layout)
{
    if (state == NULL || layout == NULL) {
        return;
    }

    const int item_x = layout->win_x + 4;
    int start_y = layout->win_y + 3;

    for (size_t i = 0; i < (size_t)APP_MENU_MAIN_COUNT; i++) {
        const char *item = app_menu_main_items[i];
        const int item_len = (int)strlen(item);
        const bool selected = ((int)i == state->sidebar_cursor);
        const uintattr_t fg = selected ? TUI_COLOR_SILVER | TB_BOLD : TUI_COLOR_BLACK;
        const uintattr_t bg = selected ? TUI_COLOR_COBALT : TUI_COLOR_SILVER;

        const int box_x = layout->win_x + 1;

        for (int x = box_x; x < box_x + item_len + 4; x++) {
            tb_set_cell(x, start_y, ' ', fg, bg);
        }

        if (selected) {
            tb_set_cell(item_x - 2, start_y, TUI_ARROW_LEFT, fg, bg);
        }

        tb_print(item_x, start_y, fg, bg, item);

        start_y++;
    }
}

void chrome_draw_divider(const layout_t *layout)
{
    if (layout == NULL) {
        return;
    }

    const int win_bottom = layout->win_y + layout->win_h;

    for (int y = layout->win_y + 1; y < win_bottom; y++) {
        tb_set_cell(layout->divider_x, y, TUI_BOX_V, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    }

    tb_set_cell(layout->divider_x, layout->win_y, TUI_BOX_BB, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    tb_set_cell(layout->divider_x, layout->win_y + 2, TUI_BOX_UB, TUI_COLOR_BLACK,
                TUI_COLOR_SILVER);
    tb_set_cell(layout->divider_x, win_bottom - 1, TUI_BOX_BU, TUI_COLOR_BLACK, TUI_COLOR_SILVER);
}
