/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef UI_H
#define UI_H

#include <stdint.h>

#include "dfa.h"
#include "input.h"

#define ARROW_L 0x2192 /* → */

#define BOX_TL 0x250C /* ┌ */
#define BOX_BL 0x2514 /* └ */

#define BOX_TR 0x2510 /* ┐ */
#define BOX_BR 0x2518 /* ┘ */

#define BOX_H 0x2500 /* ─ */
#define BOX_V 0x2502 /* │ */

#define BOX_VL 0x2524 /* ┤ */
#define BOX_VR 0x251C /* ├ */

#define BOX_BB 0x252C /* ┬ */
#define BOX_UB 0x253C /* ┼ */
#define BOX_BU 0x2534 /* ┴ */

#define COLOR_COBALT 25
#define COLOR_GREEN 82
#define COLOR_RED 196
#define COLOR_WHITE 231
#define COLOR_BLACK 232
#define COLOR_CHARCOAL 236
#define COLOR_SILVER 248

void render_input_field(int x, int y, const input_field_t *field,
                        const char *placeholder);
void render_input_status(int x, int y, const input_field_t *field,
                         const dfa_t *validator);
void render_input_error(int x, int y, const char *error_msg);

void render_section_title(int x, int y, const char *title);
void render_label_value_pair(int x, int y, int width, const char *label,
                             const char *value);
void render_bin_number(int x, int y, uint8_t number);
void render_bin_section(int x, int y, int width, const char *label,
                        uint32_t first_octet, uint8_t second_octet,
                        uint8_t third_octet, uint8_t fourth_octet);

#endif /* UI_H */
