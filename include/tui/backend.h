/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_BACKEND_H
#define VLSX_TUI_BACKEND_H

#include <stdbool.h>

#include "common/ui_event.h"

int tui_backend_init(void);
void tui_backend_shutdown(void);
int tui_backend_set_output_mode_256(void);

int tui_backend_poll(ui_event_t *event);

int tui_backend_width(void);
int tui_backend_height(void);

void tui_backend_clear(void);
void tui_backend_present(void);

#endif /* VLSX_TUI_BACKEND_H */
