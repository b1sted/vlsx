/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/ip/ipv4_screen.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/input_field.h"
#include "common/result.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "core/net/ipv4/ipv4_validate.h"

void ipv4_screen_init(ipv4_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    memset(screen, 0, sizeof(*screen));
    input_field_init(&screen->input, screen->buf, sizeof(screen->buf));
    calc_status_clear(&screen->status);
}

void ipv4_screen_confirm(ipv4_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->has_result = true;
    screen->is_valid = false;
    memset(&screen->result, 0, sizeof(screen->result));
    calc_status_clear(&screen->status);

    screen->is_valid = ipv4_calculate(screen->input.buf, &screen->result, &screen->status);
}

const input_field_t *ipv4_screen_active_input(const ipv4_screen_t *screen)
{
    if (screen == NULL) {
        return NULL;
    }

    return &screen->input;
}

bool ipv4_screen_handle(ipv4_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (event->kind) {
    case UI_EVENT_KEY_ENTER:
        ipv4_screen_confirm(screen);
        return true;
    case UI_EVENT_KEY_BACKSPACE:
        input_backspace(&screen->input);
        return true;
    case UI_EVENT_KEY_LEFT:
        input_cursor_left(&screen->input);
        return true;
    case UI_EVENT_KEY_RIGHT:
        input_cursor_right(&screen->input);
        return true;
    case UI_EVENT_KEY_UP:
    case UI_EVENT_KEY_DOWN:
        return true;
    case UI_EVENT_KEY_CTRL:
        if (event->ctrl == 'u') {
            input_field_clear(&screen->input);
        } else if (event->ctrl == 'a') {
            input_cursor_home(&screen->input);
        } else if (event->ctrl == 'e') {
            input_cursor_end(&screen->input);
        }
        return true;
    case UI_EVENT_KEY_CHAR:
        if (event->ch > 0 && event->ch <= UINT32_C(0x7F)) {
            input_insert(&screen->input, (char)event->ch, ipv4_accept);
        }
        return true;
    case UI_EVENT_NONE:
    case UI_EVENT_RESIZE:
    case UI_EVENT_KEY_ESC:
        return false;
    }

    return false;
}
