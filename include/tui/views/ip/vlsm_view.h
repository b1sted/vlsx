/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_VIEWS_IP_VLSM_VIEW_H
#define VLSX_TUI_VIEWS_IP_VLSM_VIEW_H

#include <stdbool.h>

#include "common/ui_event.h"
#include "screens/ip/vlsm_screen.h"
#include "tui/layout.h"

void vlsm_view_render(const vlsm_screen_t *screen, const layout_t *layout);
bool vlsm_view_handle_event(const vlsm_screen_t *screen, const ui_event_t *event);

#endif /* VLSX_TUI_VIEWS_IP_VLSM_VIEW_H */
