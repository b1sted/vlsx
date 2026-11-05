/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/backend.h"

#include <stdbool.h>
#include <stdint.h>

#define TB_IMPL
#include "termbox2.h"

static void map_event(const struct tb_event *source, ui_event_t *dest);

static void map_event(const struct tb_event *source, ui_event_t *dest)
{
    dest->kind = UI_EVENT_NONE;
    dest->ch = 0;
    dest->ctrl = 0;

    if (source->type == TB_EVENT_RESIZE) {
        dest->kind = UI_EVENT_RESIZE;
        return;
    }

    if (source->type != TB_EVENT_KEY) {
        return;
    }

    switch (source->key) {
    case TB_KEY_ARROW_UP:
        dest->kind = UI_EVENT_KEY_UP;
        return;
    case TB_KEY_ARROW_DOWN:
        dest->kind = UI_EVENT_KEY_DOWN;
        return;
    case TB_KEY_ARROW_LEFT:
        dest->kind = UI_EVENT_KEY_LEFT;
        return;
    case TB_KEY_ARROW_RIGHT:
        dest->kind = UI_EVENT_KEY_RIGHT;
        return;
    case TB_KEY_ENTER:
        dest->kind = UI_EVENT_KEY_ENTER;
        return;
    case TB_KEY_ESC:
        dest->kind = UI_EVENT_KEY_ESC;
        return;
    case TB_KEY_BACKSPACE:
    case TB_KEY_BACKSPACE2:
        dest->kind = UI_EVENT_KEY_BACKSPACE;
        return;
    case TB_KEY_CTRL_U:
        dest->kind = UI_EVENT_KEY_CTRL;
        dest->ctrl = (uint32_t)'u';
        return;
    case TB_KEY_CTRL_A:
        dest->kind = UI_EVENT_KEY_CTRL;
        dest->ctrl = (uint32_t)'a';
        return;
    case TB_KEY_CTRL_E:
        dest->kind = UI_EVENT_KEY_CTRL;
        dest->ctrl = (uint32_t)'e';
        return;
    case 0:
        if (source->ch != 0) {
            dest->kind = UI_EVENT_KEY_CHAR;
            dest->ch = source->ch;
        }
        return;
    default:
        return;
    }
}

int tui_backend_init(void)
{
    return tb_init();
}

void tui_backend_shutdown(void)
{
    tb_shutdown();
}

int tui_backend_set_output_mode_256(void)
{
    return tb_set_output_mode(TB_OUTPUT_256);
}

int tui_backend_poll(ui_event_t *event)
{
    struct tb_event tb_event = {0};
    const int poll_code = tb_poll_event(&tb_event);

    if (poll_code == TB_ERR || event == NULL) {
        return -1;
    }

    map_event(&tb_event, event);
    return 0;
}

int tui_backend_width(void)
{
    return tb_width();
}

int tui_backend_height(void)
{
    return tb_height();
}

void tui_backend_clear(void)
{
    tb_clear();
}

void tui_backend_present(void)
{
    tb_present();
}
