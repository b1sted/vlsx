/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_CHROME_H
#define VLSX_TUI_CHROME_H

#include "app/app.h"
#include "tui/layout.h"

void chrome_draw_background(int width, int height);
void chrome_draw_window(const layout_t *layout);
void chrome_draw_titlebar(const layout_t *layout);
void chrome_draw_main_menu(const app_state_t *state, const layout_t *layout);
void chrome_draw_divider(const layout_t *layout);

#endif /* VLSX_TUI_CHROME_H */
