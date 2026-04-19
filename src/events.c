/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "app.h"
#include "events.h"

#include "termbox2.h"

void handle_global_event(struct tb_event *ev) {
    switch (ev->key) {
    case 0:
        if (ev->ch == 'q')
            app.running = 0;
        break;

    case TB_KEY_ESC:
        app.running = false;
        break;
    }
}
