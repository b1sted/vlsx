/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_VIEWS_METRICS_GOODPUT_VIEW_H
#define VLSX_TUI_VIEWS_METRICS_GOODPUT_VIEW_H

#include "screens/metrics/goodput_screen.h"
#include "tui/layout.h"

void goodput_view_render(const goodput_screen_t *screen, const layout_t *layout);

#endif /* VLSX_TUI_VIEWS_METRICS_GOODPUT_VIEW_H */
