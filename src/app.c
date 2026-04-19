/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "app.h"

AppState app = {
    .running = true,

    .current_focus = FOCUS_SIDEBAR,
    .menu_items = 0,

    .sidebar_cursor = 0,
};

WinSizes sizes = {
    .win_w = 0,
    .win_h = 0,

    .win_x = 0,
    .win_y = 0,
};
