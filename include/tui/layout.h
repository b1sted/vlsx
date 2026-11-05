/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_LAYOUT_H
#define VLSX_TUI_LAYOUT_H

typedef struct {
    int win_x;
    int win_y;
    int win_w;
    int win_h;

    int content_x;
    int content_y;
    int content_w;
    int content_h;

    int divider_x;
    int tabbar_x;
    int tabbar_y;
} layout_t;

void layout_calculate(int term_w, int term_h, layout_t *out);

#endif /* VLSX_TUI_LAYOUT_H */
