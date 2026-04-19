/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdio.h>
#include <stdlib.h>

#include <locale.h>

#define TB_IMPL
#include "termbox2.h"

#include "app.h"
#include "events.h"
#include "render.h"

static void cleanup_terminal(void) { tb_shutdown(); }

int main(void) {
    setlocale(LC_ALL, "");

    int init_code = tb_init();
    if (init_code != TB_OK) {
        fprintf(stderr, "[ERROR] tb_init() error code: %d\n", init_code);
        return EXIT_FAILURE;
    }

    atexit(cleanup_terminal);

    tb_set_output_mode(TB_OUTPUT_256);

    struct tb_event ev;
    while (app.running) {
        if (!render())
            return EXIT_FAILURE;

        int poll_code = tb_poll_event(&ev);
        if (poll_code == TB_ERR) {
            fprintf(stderr, "[ERROR] tb_poll_event() error code: %d\n",
                    poll_code);
            return EXIT_FAILURE;
        }

        switch (ev.type) {
        case TB_EVENT_KEY:
            handle_global_event(&ev);
            break;
        case TB_EVENT_RESIZE:
            break;
        }
    }

    return EXIT_SUCCESS;
}
