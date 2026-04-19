/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef APP_H
#define APP_H

#include <stdbool.h>

#define APP_VERSION "v0.1-alpha"

typedef enum {
    FOCUS_SIDEBAR,
} FocusOn;

typedef struct {
    bool running;

    FocusOn current_focus;
    int menu_items;

    int sidebar_cursor;
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
