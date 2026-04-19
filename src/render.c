/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "app.h"
#include "render.h"

#include "termbox2.h"

#define COLOR_COBALT 25
#define COLOR_WHITE 231
#define COLOR_BLACK 232
#define COLOR_CHARCOAL 236
#define COLOR_SILVER 248

#define ARROW_L 0x2192 /* → */

#define BOX_TL 0x250C /* ┌ */
#define BOX_BL 0x2514 /* └ */

#define BOX_TR 0x2510 /* ┐ */
#define BOX_BR 0x2518 /* ┘ */

#define BOX_H 0x2500 /* ─ */
#define BOX_V 0x2502 /* │ */

#define BOX_VL 0x2524 /* ┤ */
#define BOX_VR 0x251C /* ├ */

#define BOX_BB 0x252C /* ┬ */
#define BOX_UB 0x253C /* ┼ */
#define BOX_BU 0x2534 /* ┴ */

#define MIN_WIDTH 80
#define MIN_HEIGHT 24

static const char *main_menu[] = {"1. IP & VLSM", "2. MAC Tools", "3. Metrics",
                                  "4. DHCP Timers"};

static void filing_background(const int width, const int height);

static void render_window(const int width, const int height);
static void render_outline(const int win_right, const int win_bottom);
static void render_titlebar(int *sidebar_len);
static void render_main_menu(int *sidebar_len);
static void render_divider(const int sidebar_len, const int win_bottom);

static void filing_background(const int width, const int height) {
    for (int row = 0; row < width; row++) {
        for (int col = 0; col < height; col++) {
            tb_set_cell(row, col, ' ', COLOR_COBALT, COLOR_COBALT);
        }
    }
}

static void render_window(const int width, const int height) {
    sizes.win_w = (width * 85) / 100;
    sizes.win_h = (height * 90) / 100;

    sizes.win_x = (width - sizes.win_w) / 2;
    sizes.win_y = (height - sizes.win_h) / 2;

    int win_right = sizes.win_x + sizes.win_w;
    int win_bottom = sizes.win_y + sizes.win_h;

    /* Draw a window background */
    for (int row = sizes.win_x; row < win_right; row++) {
        for (int col = sizes.win_y; col < win_bottom; col++) {
            tb_set_cell(row, col, ' ', COLOR_SILVER, COLOR_SILVER);
        }
    }

    /* Draw a horizontal shadow */
    for (int row = sizes.win_x + 2; row < win_right + 2; row++) {
        tb_set_cell(row, win_bottom, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
    }

    /* Draw a vertical shadow */
    for (int col = sizes.win_y + 1; col < win_bottom; col++) {
        tb_set_cell(win_right, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
        tb_set_cell(win_right + 1, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
    }

    int sidebar_len = 0;

    render_outline(win_right, win_bottom);
    render_titlebar(&sidebar_len);
    render_main_menu(&sidebar_len);
    render_divider(sidebar_len, win_bottom);
}

static void render_outline(const int win_right, const int win_bottom) {
    /* Corner's */
    tb_set_cell(sizes.win_x, sizes.win_y, BOX_TL, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(win_right - 1, sizes.win_y, BOX_TR, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(sizes.win_x, win_bottom - 1, BOX_BL, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(win_right - 1, win_bottom - 1, BOX_BR, COLOR_BLACK,
                COLOR_SILVER);

    /* Vertical borders */
    for (int col = sizes.win_y + 1; col < win_bottom - 1; col++) {
        tb_set_cell(sizes.win_x, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(win_right - 1, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
    }

    tb_set_cell(sizes.win_x, sizes.win_y + 2, BOX_VR, COLOR_BLACK,
                COLOR_SILVER);
    tb_set_cell(win_right - 1, sizes.win_y + 2, BOX_VL, COLOR_BLACK,
                COLOR_SILVER);

    /* Horizontal borders */
    for (int row = sizes.win_x + 1; row < win_right - 1; row++) {
        tb_set_cell(row, sizes.win_y, BOX_H, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(row, sizes.win_y + 2, BOX_H, COLOR_BLACK, COLOR_SILVER);
        tb_set_cell(row, win_bottom - 1, BOX_H, COLOR_BLACK, COLOR_SILVER);
    }
}

static void render_titlebar(int *sidebar_len) {
    const char *title = "vlsx " APP_VERSION;
    const int title_len = (int)strlen(title);
    *sidebar_len = (*sidebar_len < title_len) ? title_len : *sidebar_len;

    tb_print(sizes.win_x + 1, sizes.win_y + 1, COLOR_COBALT | TB_BOLD,
             COLOR_SILVER, title);
}

static void render_main_menu(int *sidebar_len) {
    app.menu_items = (int)(sizeof(main_menu) / sizeof(main_menu[0]));

    int item_x = sizes.win_x + 4;
    int start_y = sizes.win_y + 3;

    for (int i = 0; i < app.menu_items; i++) {
        int item_len = (int)(strlen(main_menu[i]));

        bool selected = (i == app.sidebar_cursor) ? true : false;
        uintattr_t fg = selected ? COLOR_SILVER | TB_BOLD : COLOR_BLACK;
        uintattr_t bg = selected ? COLOR_COBALT : COLOR_SILVER;

        int box_x = sizes.win_x + 1;
        for (int row = box_x; row < box_x + item_len + 4; row++) {
            tb_set_cell(row, start_y, ' ', fg, bg);
        }

        if (selected) {
            int arrow_x = item_x - 2;
            tb_set_cell(arrow_x, start_y, ARROW_L, fg, bg);
        }

        tb_print(item_x, start_y, fg, bg, main_menu[i]);

        item_len += 5;
        *sidebar_len = (*sidebar_len < item_len) ? item_len : *sidebar_len;
        start_y += 1;
    }
}

static void render_divider(const int sidebar_len, const int win_bottom) {
    int divider_x = sizes.win_x + sidebar_len;

    for (int col = sizes.win_y + 1; col < win_bottom; col++) {
        tb_set_cell(divider_x, col, BOX_V, COLOR_BLACK, COLOR_SILVER);
    }

    tb_set_cell(divider_x, sizes.win_y, BOX_BB, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(divider_x, sizes.win_y + 2, BOX_UB, COLOR_BLACK, COLOR_SILVER);
    tb_set_cell(divider_x, win_bottom - 1, BOX_BU, COLOR_BLACK, COLOR_SILVER);
}

void render(void) {
    tb_clear();

    int width = tb_width();
    int height = tb_height();

    if (width < MIN_WIDTH || height < MIN_HEIGHT) {
        const char *size_err = "Terminal is too small!";
        const int size_err_len = (int)strlen(size_err);

        int error_x = (width - size_err_len) / 2;
        int error_y = height / 2 - 1;
        tb_print(error_x, error_y, COLOR_WHITE | TB_BOLD, TB_DEFAULT, size_err);

        const char *size_info = "Minimum required terminal size: 80x24";
        const int size_info_len = (int)strlen(size_info);

        int info_x = (width - size_info_len) / 2;
        int info_y = height / 2;
        tb_print(info_x, info_y, COLOR_WHITE | TB_BOLD, TB_DEFAULT, size_info);

        tb_present();
        return;
    }

    filing_background(width, height);
    render_window(width, height);

    tb_present();

    return;
}
