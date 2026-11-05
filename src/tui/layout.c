/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/layout.h"

#include <stddef.h>
#include <string.h>

#include "app/app.h"
#include "app/menus.h"

static int layout_sidebar_width(void);

static int layout_sidebar_width(void)
{
    int width = (int)strlen("vlsx " APP_VERSION);

    for (size_t i = 0; i < (size_t)APP_MENU_MAIN_COUNT; i++) {
        const int item_width = (int)strlen(app_menu_main_items[i]) + 5;

        if (item_width > width) {
            width = item_width;
        }
    }

    return width;
}

void layout_calculate(int term_w, int term_h, layout_t *out)
{
    if (out == NULL) {
        return;
    }

    out->win_w = (term_w * 85) / 100;
    out->win_h = (term_h * 90) / 100;
    out->win_x = (term_w - out->win_w) / 2;
    out->win_y = (term_h - out->win_h) / 2;

    const int sidebar_width = layout_sidebar_width();

    out->divider_x = out->win_x + sidebar_width;
    out->tabbar_x = out->win_x + sidebar_width + 3;
    out->tabbar_y = out->win_y + 1;

    out->content_x = out->win_x + sidebar_width + 2;
    out->content_y = out->win_y + 3;
    out->content_w = out->win_w - sidebar_width - 3;
    out->content_h = out->win_h - 4;

    if (out->content_w < 0) {
        out->content_w = 0;
    }

    if (out->content_h < 0) {
        out->content_h = 0;
    }
}
