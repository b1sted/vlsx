/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "screens/metrics/metrics_screen.h"

#include <stdbool.h>
#include <stddef.h>

#include "screens/metrics/availability_screen.h"
#include "screens/metrics/costs_screen.h"
#include "screens/metrics/goodput_screen.h"

void metrics_screen_init(metrics_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->active_tab = METRICS_TAB_AVAILABILITY;
    availability_screen_init(&screen->availability);
    goodput_screen_init(&screen->goodput);
    costs_screen_init(&screen->costs);
}

void metrics_screen_set_tab(metrics_screen_t *screen, size_t tab_index)
{
    if (screen == NULL || tab_index >= (size_t)METRICS_TAB_COUNT) {
        return;
    }

    screen->active_tab = (metrics_tab_t)tab_index;
}

metrics_tab_t metrics_screen_active_tab(const metrics_screen_t *screen)
{
    if (screen == NULL) {
        return METRICS_TAB_AVAILABILITY;
    }

    return screen->active_tab;
}

void metrics_screen_focus(metrics_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    screen->availability.active_input = 0;
    screen->goodput.active_input = 0;
    screen->costs.active_input = 0;
}

void metrics_screen_confirm(metrics_screen_t *screen)
{
    if (screen == NULL) {
        return;
    }

    switch (screen->active_tab) {
    case METRICS_TAB_AVAILABILITY:
        availability_screen_confirm(&screen->availability);
        break;
    case METRICS_TAB_GOODPUT:
        goodput_screen_confirm(&screen->goodput);
        break;
    case METRICS_TAB_COSTS:
        costs_screen_confirm(&screen->costs);
        break;
    case METRICS_TAB_COUNT:
        break;
    }
}

bool metrics_screen_handle(metrics_screen_t *screen, const ui_event_t *event)
{
    if (screen == NULL || event == NULL) {
        return false;
    }

    switch (screen->active_tab) {
    case METRICS_TAB_AVAILABILITY:
        return availability_screen_handle(&screen->availability, event);
    case METRICS_TAB_GOODPUT:
        return goodput_screen_handle(&screen->goodput, event);
    case METRICS_TAB_COSTS:
        return costs_screen_handle(&screen->costs, event);
    case METRICS_TAB_COUNT:
        return false;
    }

    return false;
}
