/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_APP_APP_H
#define VLSX_APP_APP_H

#include <stdbool.h>

#include "screens/ip/ip_screen.h"
#include "screens/l2/mac_screen.h"
#include "screens/metrics/metrics_screen.h"

#define APP_VERSION "v0.1-alpha"

typedef enum {
    FOCUS_SIDEBAR,
    FOCUS_TABBAR,
    FOCUS_INPUT,
} focus_on_t;

typedef enum {
    APP_SCR_MAIN,
    APP_SCR_IP,
    APP_SCR_MAC,
    APP_SCR_METRICS,
} screens_t;

typedef struct {
    focus_on_t current_focus;
    focus_on_t previous_focus;

    screens_t current_screen;
    screens_t previous_screen;

    int sidebar_cursor;
    int tabbar_cursor;
    int mac_tabbar_cursor;
    int metrics_tabbar_cursor;

    bool running;

    ip_screen_t ip_screen;
    mac_screen_t mac_screen;
    metrics_screen_t metrics_screen;
} app_state_t;

extern app_state_t app;

void app_init(void);

#endif /* VLSX_APP_APP_H */
