/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_TUI_TABLE_H
#define VLSX_TUI_TABLE_H

#include <stdbool.h>

typedef enum {
    TABLE_ALIGN_LEFT,
    TABLE_ALIGN_RIGHT,
    TABLE_ALIGN_CENTER,
} table_align_t;

typedef struct {
    const char *title;
    int width;
    table_align_t align;
} table_column_t;

typedef struct {
    const table_column_t *columns;
    int column_count;
    const char *const *cells;
    int row_count;
} table_t;

int table_total_width(const table_t *table);
int table_total_height(const table_t *table, int height);
int table_max_row_offset(const table_t *table, int height);
int table_max_column_offset(const table_t *table, int width);

void table_render(int x, int y, int width, int height, const table_t *table, int row_offset,
                  int column_offset);

#endif /* VLSX_TUI_TABLE_H */
