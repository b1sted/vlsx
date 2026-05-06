/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>

#include "termbox2.h"

#include "dfa.h"
#include "input.h"
#include "ui.h"

void render_input_field(int x, int y, const input_field_t *field,
                        const char *placeholder) {
    bool has_input = field->len > 0;

    uintattr_t fg = has_input ? COLOR_BLACK : COLOR_COBALT;
    tb_printf(x, y, fg, COLOR_SILVER, has_input ? field->buf : placeholder);
    tb_set_cursor(x + field->cursor, y);
}

void render_input_status(int x, int y, const input_field_t *field,
                         const dfa_t *validator) {
    if (field->len <= 0)
        return;

    bool syntax_ok = run_dfa(validator, field->buf);
    tb_printf(x, y, syntax_ok ? COLOR_GREEN : COLOR_RED, COLOR_SILVER,
              syntax_ok ? "✓ Valid characters" : "");
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
                        uint32_t first_octet, uint8_t second_octet,
                        uint8_t third_octet, uint8_t fourth_octet) {
    tb_print(x, y, COLOR_BLACK | TB_BOLD, COLOR_SILVER, label);

    int start_x = x + width;
    render_bin_number(start_x, y, first_octet);
    tb_set_cell(start_x + 8, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 9, y, second_octet);
    tb_set_cell(start_x + 17, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 18, y, third_octet);
    tb_set_cell(start_x + 26, y, '.', COLOR_BLACK, COLOR_SILVER);

    render_bin_number(start_x + 27, y, fourth_octet);
}
