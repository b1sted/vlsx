/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/ip/ipv6_screen.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/input_field.h"
#include "common/result.h"
#include "core/net/ipv6/ipv6_calc.h"
#include "core/net/ipv6/ipv6_validate.h"

void ipv6_screen_init(ipv6_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    memset(screen, 0, sizeof(*screen));
    input_field_init(&screen->input, screen->buf, sizeof(screen->buf));
    calc_status_clear(&screen->status);
}

void ipv6_screen_confirm(ipv6_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->has_result = true;
    screen->is_valid = false;
    memset(&screen->result, 0, sizeof(screen->result));
    calc_status_clear(&screen->status);

    screen->is_valid = ipv6_calculate(screen->input.buf, &screen->result, &screen->status);
}

const input_field_t *ipv6_screen_active_input(const ipv6_screen_t *screen)
{
    if (screen == NULL) {
        return NULL;
    }

    return &screen->input;
}

bool ipv6_screen_handle(ipv6_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (event->kind) {
    case UI_EVENT_KEY_ENTER:
        ipv6_screen_confirm(screen);
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
            input_insert(&screen->input, (char)event->ch, ipv6_accept);
        }
        return true;
    case UI_EVENT_NONE:
    case UI_EVENT_RESIZE:
    case UI_EVENT_KEY_ESC:
        return false;
    }

    return false;
}
