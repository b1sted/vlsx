/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "app.h"
#include "render.h"

#include "termbox2.h"

#define COLOR_COBALT 25
#define COLOR_SILVER 248
#define COLOR_CHARCOAL 236

static void filing_background(int width, int height);

static void render_window(int width, int height);

static void filing_background(int width, int height) {
    for (int row = 0; row < width; row++) {
        for (int col = 0; col < height; col++) {
            tb_set_cell(row, col, ' ', COLOR_COBALT, COLOR_COBALT);
        }
    }
}

static void render_window(int width, int height) {
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

    /* Draw a horizontal shadow */
    for (int col = sizes.win_y + 1; col < win_bottom; col++) {
        tb_set_cell(win_right, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
        tb_set_cell(win_right + 1, col, ' ', COLOR_CHARCOAL, COLOR_CHARCOAL);
    }
}

bool render(void) {
    tb_clear();

    int width = tb_width();
    int height = tb_height();

    filing_background(width, height);
    render_window(width, height);

    tb_present();

    return true;
}
