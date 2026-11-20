/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/app.h"
#include "app/events.h"
#include "common/ui_event.h"
#include "screens/ip/ip_screen.h"
#include "tui/backend.h"
#include "tui/render.h"
#include "tui/views/ip/vlsm_view.h"

#ifndef VLSX_VERSION
#define VLSX_VERSION "dev"
#endif

static bool vlsm_view_is_active(void);

static bool vlsm_view_is_active(void)
{
    return app.current_screen == APP_SCR_IP && app.current_focus == FOCUS_INPUT &&
           ip_screen_active_tab(&app.ip_screen) == IP_TAB_VLSM;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            printf("vlsx %s\n", VLSX_VERSION);
            return EXIT_SUCCESS;
        }

        fprintf(stderr, "Usage: %s [-v|--version]\n", argv[0]);
        return EXIT_FAILURE;
    }

    setlocale(LC_ALL, "");

    const int init_code = tui_backend_init();

    if (init_code != 0) {
        fprintf(stderr, "[ERROR] tui_backend_init() error code: %d\n", init_code);
        return EXIT_FAILURE;
    }

    if (atexit(tui_backend_shutdown) != 0) {
        fprintf(stderr, "[ERROR] atexit() registration failed\n");
        tui_backend_shutdown();
        return EXIT_FAILURE;
    }

    const int mode_code = tui_backend_set_output_mode_256();

    if (mode_code != 0) {
        fprintf(stderr,
                "[ERROR] tui_backend_set_output_mode_256() error "
                "code: %d\n",
                mode_code);
        return EXIT_FAILURE;
    }

    app_init();

    while (app.running) {
        render_frame();

        ui_event_t event = {0};

        if (tui_backend_poll(&event) != 0) {
            fprintf(stderr, "[ERROR] tui_backend_poll() failed\n");
            return EXIT_FAILURE;
        }

        if (event.kind == UI_EVENT_RESIZE) {
            continue;
        }

        bool consumed = false;

        if (vlsm_view_is_active()) {
            consumed = vlsm_view_handle_event(&app.ip_screen.vlsm, &event);
        }

        if (!consumed) {
            app_handle_event(&event);
        }
    }

    return EXIT_SUCCESS;
}
