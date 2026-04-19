/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef EVENTS_H
#define EVENTS_H

struct tb_event;

void handle_global_event(struct tb_event *ev);

#endif /* EVENTS_H */
