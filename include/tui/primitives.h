/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_PRIMITIVES_H
#define VLSX_TUI_PRIMITIVES_H

#include <stdbool.h>
#include <stdint.h>

#include "common/input_field.h"

void ui_render_input_error(int x, int y, const char *error_msg);
void ui_render_input_status(int x, int y, const input_field_t *field,
                            bool (*validate)(const char *));
void ui_render_input_block(int x, int y, int width, const char *label, int post_colon_spacing,
                           const input_field_t *field, const char *placeholder,
                           bool (*validate)(const char *), bool is_active);
void ui_render_input_cursor(int x, int y, const input_field_t *field, const char *placeholder);
void ui_render_set_input_focus(bool focused);

void ui_render_section_title(int x, int y, const char *title);
void ui_render_label_value_pair(int x, int y, int width, const char *label, const char *value);
void ui_render_bin_number(int x, int y, uint8_t number);
void ui_render_bin_section(int x, int y, int width, const char *label,
                           const uint8_t octets_arr[static 4]);

#endif /* VLSX_TUI_PRIMITIVES_H */
