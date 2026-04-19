/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef APP_H
#define APP_H

#include <stdbool.h>

typedef struct {
    bool running;
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
