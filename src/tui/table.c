/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "tui/table.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "termbox2.h"

#include "tui/theme.h"

static int table_text_padding(int field_width, int text_len, table_align_t align);
static int table_column_width(const table_t *table, int column_index);
static void table_draw_char(int x, int y, int width, int height, int row_y, int logical_x,
                            int column_offset, uint32_t ch, uintattr_t fg, uintattr_t bg);
static void table_draw_field(int x, int y, int width, int height, int row_y, int logical_x,
                             int column_offset, const char *text, int field_width,
                             table_align_t align, uintattr_t fg, uintattr_t bg);
static void table_draw_row(int x, int y, int width, int height, int row_y, int column_offset,
                           const table_t *table, const char *const *cells, bool header,
                           uintattr_t fg, uintattr_t bg);
static void table_draw_divider(int x, int y, int width, int height, int row_y, int column_offset,
                               const table_t *table);
static int table_body_height(int height);

static int table_text_padding(int field_width, int text_len, table_align_t align)
{
    const int padding = field_width - text_len;

    if (padding <= 0) {
        return 0;
    }

    switch (align) {
    case TABLE_ALIGN_RIGHT:
        return padding;
    case TABLE_ALIGN_CENTER:
        return padding / 2;
    case TABLE_ALIGN_LEFT:
    default:
        return 0;
    }
}

static int table_column_width(const table_t *table, int column_index)
{
    const table_column_t *column = &table->columns[column_index];
    int width = (column->width > 0) ? column->width : 0;

    if (column->title != NULL) {
        const int title_len = (int)strlen(column->title);

        if (title_len > width) {
            width = title_len;
        }
    }

    if (table->cells == NULL) {
        return width;
    }

    for (int row = 0; row < table->row_count; row++) {
        const char *cell =
            table->cells[(size_t)row * (size_t)table->column_count + (size_t)column_index];

        if (cell == NULL) {
            continue;
        }

        const int cell_len = (int)strlen(cell);

        if (cell_len > width) {
            width = cell_len;
        }
    }

    return width;
}

static void table_draw_char(int x, int y, int width, int height, int row_y, int logical_x,
                            int column_offset, uint32_t ch, uintattr_t fg, uintattr_t bg)
{
    if (row_y < y || row_y >= y + height) {
        return;
    }

    const int draw_x = x + logical_x - column_offset;

    if (draw_x < x || draw_x >= x + width) {
        return;
    }

    tb_set_cell(draw_x, row_y, ch, fg, bg);
}

static void table_draw_field(int x, int y, int width, int height, int row_y, int logical_x,
                             int column_offset, const char *text, int field_width,
                             table_align_t align, uintattr_t fg, uintattr_t bg)
{
    if (field_width <= 0) {
        return;
    }

    if (text == NULL) {
        text = "";
    }

    const int text_len = (int)strlen(text);
    const int padding = table_text_padding(field_width, text_len, align);

    for (int i = 0; i < field_width; i++) {
        const int text_index = i - padding;
        const char ch = (text_index >= 0 && text_index < text_len) ? text[text_index] : ' ';

        table_draw_char(x, y, width, height, row_y, logical_x + i, column_offset,
                        (uint32_t)(unsigned char)ch, fg, bg);
    }
}

static void table_draw_row(int x, int y, int width, int height, int row_y, int column_offset,
                           const table_t *table, const char *const *cells, bool header,
                           uintattr_t fg, uintattr_t bg)
{
    int logical_x = 0;

    for (int i = 0; i < table->column_count; i++) {
        const table_column_t *column = &table->columns[i];
        const char *text = header ? column->title : cells[i];
        const int field_width = table_column_width(table, i);

        table_draw_field(x, y, width, height, row_y, logical_x, column_offset, text, field_width,
                         column->align, fg, bg);

        logical_x += field_width;

        if (i < table->column_count - 1) {
            table_draw_char(x, y, width, height, row_y, logical_x, column_offset, '|', fg, bg);
            logical_x++;
        }
    }
}

static void table_draw_divider(int x, int y, int width, int height, int row_y, int column_offset,
                               const table_t *table)
{
    const int total_width = table_total_width(table);

    for (int i = 0; i < total_width; i++) {
        table_draw_char(x, y, width, height, row_y, i, column_offset, '-', TUI_COLOR_CHARCOAL,
                        TUI_COLOR_SILVER);
    }
}

static int table_body_height(int height)
{
    return (height > 2) ? height - 2 : 0;
}

int table_total_width(const table_t *table)
{
    if (table == NULL || table->columns == NULL || table->column_count <= 0) {
        return 0;
    }

    int width = table->column_count - 1;

    for (int i = 0; i < table->column_count; i++) {
        width += table_column_width(table, i);
    }

    return width;
}

int table_total_height(const table_t *table, int height)
{
    if (table == NULL || table->columns == NULL || table->column_count <= 0 || height <= 0) {
        return 0;
    }

    int needed = 2; /* header + divider */

    if (table->cells != NULL && table->row_count > 0) {
        needed += table->row_count;
    }

    return (needed < height) ? needed : height;
}

int table_max_row_offset(const table_t *table, int height)
{
    if (table == NULL) {
        return 0;
    }

    const int body_height = table_body_height(height);

    if (body_height <= 0 || table->row_count <= body_height) {
        return 0;
    }

    return table->row_count - body_height;
}

int table_max_column_offset(const table_t *table, int width)
{
    const int total_width = table_total_width(table);

    if (width <= 0 || total_width <= width) {
        return 0;
    }

    return total_width - width;
}

void table_render(int x, int y, int width, int height, const table_t *table, int row_offset,
                  int column_offset)
{
    if (table == NULL || table->columns == NULL || table->column_count <= 0) {
        return;
    }

    if (width <= 0 || height <= 0) {
        return;
    }

    if (row_offset < 0) {
        row_offset = 0;
    } else if (row_offset > table_max_row_offset(table, height)) {
        row_offset = table_max_row_offset(table, height);
    }

    if (column_offset < 0) {
        column_offset = 0;
    } else if (column_offset > table_max_column_offset(table, width)) {
        column_offset = table_max_column_offset(table, width);
    }

    table_draw_row(x, y, width, height, y, column_offset, table, NULL, true,
                   TUI_COLOR_COBALT | TB_BOLD, TUI_COLOR_SILVER);

    if (height < 2) {
        return;
    }

    table_draw_divider(x, y, width, height, y + 1, column_offset, table);

    if (table->cells == NULL || table->row_count <= 0) {
        return;
    }

    const int body_height = table_body_height(height);

    for (int i = 0; i < body_height; i++) {
        const int row = row_offset + i;

        if (row >= table->row_count) {
            break;
        }

        const char *const *cells = table->cells + ((size_t)row * (size_t)table->column_count);

        table_draw_row(x, y, width, height, y + 2 + i, column_offset, table, cells, false,
                       TUI_COLOR_BLACK, TUI_COLOR_SILVER);
    }
}
