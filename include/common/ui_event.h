/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_UI_EVENT_H
#define VLSX_COMMON_UI_EVENT_H

#include <stdint.h>

typedef enum {
    UI_EVENT_NONE,
    UI_EVENT_RESIZE,
    UI_EVENT_KEY_UP,
    UI_EVENT_KEY_DOWN,
    UI_EVENT_KEY_LEFT,
    UI_EVENT_KEY_RIGHT,
    UI_EVENT_KEY_ENTER,
    UI_EVENT_KEY_ESC,
    UI_EVENT_KEY_BACKSPACE,
    UI_EVENT_KEY_CHAR,
    UI_EVENT_KEY_CTRL,
} ui_event_kind_t;

typedef struct {
    ui_event_kind_t kind;
    uint32_t ch;
    uint32_t ctrl;
} ui_event_t;

#endif /* VLSX_COMMON_UI_EVENT_H */
