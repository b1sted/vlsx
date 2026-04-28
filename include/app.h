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
} FocusOn;

typedef enum {
    APP_SCR_MAIN,
    APP_SCR_IP,
} Screens;

typedef struct {
    InputField *active_field;
    bool (*active_accept)(char);

    FocusOn current_focus;
    FocusOn previous_focus;

    Screens current_screen;
    Screens previous_screen;

    int sidebar_items;
    int tabbar_items;
    int input_count;

    int sidebar_cursor;
    int tabbar_cursor;
    int input_cursor;

    bool running;
    bool has_press_input;
    bool is_input_result_valid;
} AppState;

typedef struct {
    int win_w;
    int win_h;

    int win_x;
    int win_y;
} WinSizes;

extern AppState app;
extern WinSizes sizes;

#endif /* APP_H */
