/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stddef.h>

#include "app.h"

app_state_t app = {
    .active_field = NULL,
    .active_accept = NULL,

    .current_focus = FOCUS_SIDEBAR,
    .previous_focus = FOCUS_SIDEBAR,

    .current_screen = APP_SCR_MAIN,
    .previous_screen = APP_SCR_MAIN,

    .sidebar_items = 0,
    .tabbar_items = 0,
    .input_count = 0,

    .sidebar_cursor = 0,
    .tabbar_cursor = 0,
    .input_cursor = 0,

    .running = true,
    .has_press_input = false,
    .is_input_result_valid = false,
};

win_sizes_t sizes = {
    .win_w = 0,
    .win_h = 0,

    .win_x = 0,
    .win_y = 0,
};
