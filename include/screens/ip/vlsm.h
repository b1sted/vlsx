/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSM_H
#define VLSM_H

#include <stdbool.h>

struct tb_event;

void vlsm_focus_change(void);
void vlsm_input_confirm(void);
bool vlsm_handle_event(struct tb_event *ev);
void render_vlsm_content(const int start_x, const int start_y);

#endif /* VLSM_H */
