/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_SCREENS_METRICS_METRICS_SCREEN_H
#define VLSX_SCREENS_METRICS_METRICS_SCREEN_H

#include <stdbool.h>
#include <stddef.h>

#include "common/ui_event.h"
#include "screens/metrics/availability_screen.h"
#include "screens/metrics/costs_screen.h"
#include "screens/metrics/goodput_screen.h"

typedef enum {
    METRICS_TAB_AVAILABILITY,
    METRICS_TAB_GOODPUT,
    METRICS_TAB_COSTS,
    METRICS_TAB_COUNT,
} metrics_tab_t;

typedef struct {
    metrics_tab_t active_tab;
    availability_screen_t availability;
    goodput_screen_t goodput;
    costs_screen_t costs;
} metrics_screen_t;

void metrics_screen_init(metrics_screen_t *screen);
void metrics_screen_set_tab(metrics_screen_t *screen, size_t tab_index);
metrics_tab_t metrics_screen_active_tab(const metrics_screen_t *screen);
void metrics_screen_focus(metrics_screen_t *screen);
void metrics_screen_confirm(metrics_screen_t *screen);
bool metrics_screen_handle(metrics_screen_t *screen, const ui_event_t *event);

#endif /* VLSX_SCREENS_METRICS_METRICS_SCREEN_H */
