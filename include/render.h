/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef RENDER_H
#define RENDER_H

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

void render_table(int x, int y, int width, int height, const table_t *table,
                  int row_offset, int column_offset);

void render(void);

#endif /* RENDER_H */
