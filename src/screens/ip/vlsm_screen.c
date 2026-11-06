/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/ip/vlsm_screen.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/input_field.h"
#include "common/result.h"
#include "core/net/ipv4/ipv4_validate.h"
#include "core/net/vlsm/hosts_list.h"
#include "core/net/vlsm/vlsm_alloc.h"

void vlsm_screen_init(vlsm_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    memset(screen, 0, sizeof(*screen));
    input_field_init(&screen->fields[0], screen->cidr_buf, sizeof(screen->cidr_buf));
    input_field_init(&screen->fields[1], screen->hosts_buf, sizeof(screen->hosts_buf));
    calc_status_clear(&screen->status);
}

void vlsm_screen_confirm(vlsm_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->has_result = true;
    screen->is_valid = false;
    screen->generation++;
    memset(&screen->result, 0, sizeof(screen->result));
    calc_status_clear(&screen->status);

    screen->is_valid =
        vlsm_plan(screen->fields[0].buf, screen->fields[1].buf, &screen->result, &screen->status);
}

const input_field_t *vlsm_screen_active_input(const vlsm_screen_t *screen)
{
    if (screen == NULL) {
        return NULL;
    }

    return &screen->fields[screen->active_input];
}

bool vlsm_screen_handle(vlsm_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    if (screen->active_input >= 2) {
        screen->active_input = 0;
    }

    input_field_t *field = &screen->fields[screen->active_input];

    switch (event->kind) {
    case UI_EVENT_KEY_ENTER:
        vlsm_screen_confirm(screen);
        return true;
    case UI_EVENT_KEY_BACKSPACE:
        input_backspace(field);
        return true;
    case UI_EVENT_KEY_LEFT:
        input_cursor_left(field);
        return true;
    case UI_EVENT_KEY_RIGHT:
        input_cursor_right(field);
        return true;
    case UI_EVENT_KEY_UP:
        if (screen->active_input > 0) {
            screen->active_input--;
        }
        return true;
    case UI_EVENT_KEY_DOWN:
        if (screen->active_input < 1) {
            screen->active_input++;
        }
        return true;
    case UI_EVENT_KEY_CTRL:
        if (event->ctrl == 'u') {
            input_field_clear(field);
        } else if (event->ctrl == 'a') {
            input_cursor_home(field);
        } else if (event->ctrl == 'e') {
            input_cursor_end(field);
        }
        return true;
    case UI_EVENT_KEY_CHAR:
        if (event->ch > 0 && event->ch <= UINT32_C(0x7F)) {
            bool (*accept)(char) = (screen->active_input == 0) ? ipv4_accept : hosts_accept;
            input_insert(field, (char)event->ch, accept);
        }
        return true;
    case UI_EVENT_NONE:
    case UI_EVENT_RESIZE:
    case UI_EVENT_KEY_ESC:
    case UI_EVENT_KEY_TAB:
        return false;
    }

    return false;
}
