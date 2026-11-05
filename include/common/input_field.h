/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_INPUT_FIELD_H
#define VLSX_COMMON_INPUT_FIELD_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char *buf;
    size_t cap;
    size_t len;
    size_t cursor;
} input_field_t;

void input_field_init(input_field_t *field, char *buf, size_t cap);
void input_insert(input_field_t *field, char ch, bool (*accept)(char));
void input_backspace(input_field_t *field);
void input_field_clear(input_field_t *field);
void input_cursor_left(input_field_t *field);
void input_cursor_right(input_field_t *field);
void input_cursor_home(input_field_t *field);
void input_cursor_end(input_field_t *field);

#endif /* VLSX_COMMON_INPUT_FIELD_H */
