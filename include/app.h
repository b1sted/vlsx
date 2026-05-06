/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef APP_H
#define APP_H

#include <stdbool.h>

#include "input.h"

#define APP_VERSION "v0.1-alpha"

typedef enum {
    FOCUS_SIDEBAR,
    FOCUS_TABBAR,
    FOCUS_INPUT,
} focus_on_t;

typedef enum {
    APP_SCR_MAIN,
    APP_SCR_IP,
} screens_t;

typedef struct {
    input_field_t *active_field;
    bool (*active_accept)(char);

    focus_on_t current_focus;
    focus_on_t previous_focus;

    screens_t current_screen;
    screens_t previous_screen;

    int sidebar_items;
    int tabbar_items;
    int input_count;

    int sidebar_cursor;
    int tabbar_cursor;
    int input_cursor;

    bool running;
    bool has_press_input;
    bool is_input_result_valid;
} app_state_t;

typedef struct {
    int win_w;
    int win_h;

    int win_x;
    int win_y;
} win_sizes_t;

extern app_state_t app;
extern win_sizes_t sizes;

#endif /* APP_H */
