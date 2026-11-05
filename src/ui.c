/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>

#include "termbox2.h"

#include "input.h"
#include "ui.h"

static void render_input_field(int x, int y, const input_field_t *field,
                               const char *placeholder, bool is_active);
static void render_input_status(int x, int y, const input_field_t *field,
                                bool (*validate)(const char *));

static void render_input_field(int x, int y, const input_field_t *field,
                               const char *placeholder, bool is_active) {
    bool has_input = field->len > 0;

    uintattr_t fg = has_input ? COLOR_BLACK : COLOR_COBALT;
    tb_printf(x, y, fg, COLOR_SILVER, has_input ? field->buf : placeholder);
    if (is_active)
        tb_set_cursor(x + field->cursor, y);
}

static void render_input_status(int x, int y, const input_field_t *field,
                                bool (*validate)(const char *)) {
    if (field->len <= 0)
        return;

    bool syntax_ok = validate(field->buf);
    tb_printf(x, y, syntax_ok ? COLOR_GREEN : COLOR_RED, COLOR_SILVER,
              syntax_ok ? "✓ Syntax Ok" : "✗ Invalid Syntax");
}

void render_input_block(int x, int y, int width, const char *label,
                        int post_colon_spacing, const input_field_t *field,
                        const char *placeholder, bool (*validate)(const char *),
                        bool is_active) {
    tb_print(x, y, COLOR_BLACK | TB_BOLD, COLOR_SILVER, label);

    const int label_len = (int)strlen(label);
    int input_x = x + label_len + post_colon_spacing;

    render_input_field(input_x, y, field, placeholder, is_active);
    render_input_status(input_x + width, y, field, validate);
}

void render_input_error(int x, int y, const char *error_msg) {
    tb_print(x, y, COLOR_RED | TB_BOLD, COLOR_SILVER, error_msg);
}

void render_section_title(int x, int y, const char *title) {
    tb_print(x, y, COLOR_COBALT | TB_BOLD, COLOR_SILVER, title);
}

void render_label_value_pair(int x, int y, int width, const char *label,
                             const char *value) {
    tb_print(x, y, COLOR_BLACK | TB_BOLD, COLOR_SILVER, label);
    tb_print(x + width, y, COLOR_BLACK, COLOR_SILVER, value);
}

void render_bin_number(int x, int y, uint8_t number) {
    int number_x = x + 7;
    for (int i = 7; i >= 0; i--) {
        tb_printf(number_x - i, y, COLOR_BLACK, COLOR_SILVER, "%d",
                  (number >> i) & 1);
    }
}

void render_bin_section(int x, int y, int width, const char *label,
                        const uint8_t octets_arr[static 4]) {
    tb_print(x, y, COLOR_BLACK | TB_BOLD, COLOR_SILVER, label);

    int start_x = x + width;
    render_bin_number(start_x, y, octets_arr[0]);
    tb_set_cell(start_x + 8, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 9, y, octets_arr[1]);
    tb_set_cell(start_x + 17, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 18, y, octets_arr[2]);
    tb_set_cell(start_x + 26, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 27, y, octets_arr[3]);
}
