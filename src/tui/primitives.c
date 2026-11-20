/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/primitives.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "termbox2.h"

#include "common/input_field.h"
#include "tui/theme.h"

static void render_input_field(int x, int y, const input_field_t *field, const char *placeholder,
                               bool is_active);

static bool input_focused = false;

void ui_render_set_input_focus(bool focused)
{
    input_focused = focused;
}

void ui_render_input_cursor(int x, int y, const input_field_t *field, const char *placeholder)
{
    if (field == NULL || !input_focused) {
        return;
    }

    char under_cursor = ' ';

    if (field->buf != NULL && field->cursor < field->len) {
        under_cursor = field->buf[field->cursor];
    } else if (field->len == 0 && placeholder != NULL && field->cursor < strlen(placeholder)) {
        under_cursor = placeholder[field->cursor];
    }

    tb_set_cell(x + (int)field->cursor, y, (uint32_t)(unsigned char)under_cursor, TUI_COLOR_SILVER,
                TUI_COLOR_BLACK);
}

static void render_input_field(int x, int y, const input_field_t *field, const char *placeholder,
                               bool is_active)
{
    if (field == NULL) {
        return;
    }

    const bool has_input = field->len > 0;
    const uintattr_t fg = has_input ? TUI_COLOR_BLACK : TUI_COLOR_COBALT;
    const char *text = has_input ? field->buf : placeholder;

    if (text != NULL) {
        tb_print(x, y, fg, TUI_COLOR_SILVER, text);
    }

    if (is_active) {
        ui_render_input_cursor(x, y, field, placeholder);
    }
}

void ui_render_input_status(int x, int y, const input_field_t *field,
                            bool (*validate)(const char *))
{
    if (field == NULL || validate == NULL || field->len <= 0) {
        return;
    }

    const bool syntax_ok = validate(field->buf);

    tb_print(x, y, syntax_ok ? TUI_COLOR_GREEN : TUI_COLOR_RED, TUI_COLOR_SILVER,
             syntax_ok ? "✓ Syntax OK" : "✗ Invalid Syntax");
}

void ui_render_input_block(int x, int y, int width, int label_width, const char *label,
                           const input_field_t *field, const char *placeholder,
                           bool (*validate)(const char *), bool is_active)
{
    if (label == NULL || field == NULL) {
        return;
    }

    tb_print(x, y, TUI_COLOR_BLACK | TB_BOLD, TUI_COLOR_SILVER, label);

    const int input_x = x + label_width + TUI_LABEL_VALUE_GAP;

    render_input_field(input_x, y, field, placeholder, is_active);
    ui_render_input_status(input_x + width, y, field, validate);
}

void ui_render_input_error(int x, int y, const char *error_msg)
{
    if (error_msg == NULL || error_msg[0] == '\0') {
        return;
    }

    tb_print(x, y, TUI_COLOR_RED | TB_BOLD, TUI_COLOR_SILVER, error_msg);
}

void ui_render_section_title(int x, int y, const char *title)
{
    if (title == NULL) {
        return;
    }

    tb_print(x, y, TUI_COLOR_COBALT | TB_BOLD, TUI_COLOR_SILVER, title);
}

void ui_render_label_value_pair(int x, int y, int width, const char *label, const char *value)
{
    if (label == NULL || value == NULL) {
        return;
    }

    tb_print(x, y, TUI_COLOR_BLACK | TB_BOLD, TUI_COLOR_SILVER, label);
    tb_print(x + width + TUI_LABEL_VALUE_GAP, y, TUI_COLOR_BLACK, TUI_COLOR_SILVER, value);
}

void ui_render_bin_number(int x, int y, uint8_t number)
{
    const int number_x = x + 7;

    for (int i = 7; i >= 0; i--) {
        tb_printf(number_x - i, y, TUI_COLOR_BLACK, TUI_COLOR_SILVER, "%d", (number >> i) & 1);
    }
}

void ui_render_bin_section(int x, int y, int width, const char *label,
                           const uint8_t octets_arr[static 4])
{
    if (label == NULL) {
        return;
    }

    tb_print(x, y, TUI_COLOR_BLACK | TB_BOLD, TUI_COLOR_SILVER, label);

    const int start_x = x + width;

    ui_render_bin_number(start_x, y, octets_arr[0]);
    tb_set_cell(start_x + 8, y, '.', TUI_COLOR_BLACK, TUI_COLOR_SILVER);

    ui_render_bin_number(start_x + 9, y, octets_arr[1]);
    tb_set_cell(start_x + 17, y, '.', TUI_COLOR_BLACK, TUI_COLOR_SILVER);

    ui_render_bin_number(start_x + 18, y, octets_arr[2]);
    tb_set_cell(start_x + 26, y, '.', TUI_COLOR_BLACK, TUI_COLOR_SILVER);

    ui_render_bin_number(start_x + 27, y, octets_arr[3]);
}
