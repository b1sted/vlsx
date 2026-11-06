/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "app/app.h"

#include <stdbool.h>
#include <string.h>

#include "screens/ip/ip_screen.h"
#include "screens/l2/mac_screen.h"

app_state_t app = {
    .current_focus = FOCUS_SIDEBAR,
    .previous_focus = FOCUS_SIDEBAR,
    .current_screen = APP_SCR_MAIN,
    .previous_screen = APP_SCR_MAIN,
    .sidebar_cursor = 0,
    .tabbar_cursor = 0,
    .mac_tabbar_cursor = 0,
    .running = true,
};

void app_init(void)
{
    memset(&app, 0, sizeof(app));

    app.current_focus = FOCUS_SIDEBAR;
    app.previous_focus = FOCUS_SIDEBAR;
    app.current_screen = APP_SCR_MAIN;
    app.previous_screen = APP_SCR_MAIN;
    app.running = true;

    ip_screen_init(&app.ip_screen);
    mac_screen_init(&app.mac_screen);
}
