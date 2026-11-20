/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "common/input_field.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void input_field_init(input_field_t *field, char *buf, size_t cap)
{
    if (field == NULL) {
        return;
    }

    field->buf = buf;
    field->cap = (buf != NULL) ? cap : 0;
    field->len = 0;
    field->cursor = 0;

    if (field->buf != NULL && field->cap > 0) {
        field->buf[0] = '\0';
    }
}

void input_insert(input_field_t *field, char ch, bool (*accept)(char))
{
    if (field == NULL || accept == NULL || field->buf == NULL) {
        return;
    }

    if (!accept(ch)) {
        return;
    }

    if (field->len + 1 >= field->cap) {
        return;
    }

    memmove(&field->buf[field->cursor + 1], &field->buf[field->cursor], field->len - field->cursor);

    field->buf[field->cursor] = ch;
    field->len++;
    field->cursor++;
    field->buf[field->len] = '\0';
}

void input_backspace(input_field_t *field)
{
    if (field == NULL || field->buf == NULL || field->cursor == 0) {
        return;
    }

    memmove(&field->buf[field->cursor - 1], &field->buf[field->cursor], field->len - field->cursor);

    field->len--;
    field->cursor--;
    field->buf[field->len] = '\0';
}

void input_field_clear(input_field_t *field)
{
    if (field == NULL) {
        return;
    }

    if (field->buf != NULL) {
        field->buf[0] = '\0';
    }

    field->cursor = 0;
    field->len = 0;
}

void input_cursor_left(input_field_t *field)
{
    if (field != NULL && field->cursor > 0) {
        field->cursor--;
    }
}

void input_cursor_right(input_field_t *field)
{
    if (field != NULL && field->cursor < field->len) {
        field->cursor++;
    }
}

void input_cursor_home(input_field_t *field)
{
    if (field != NULL) {
        field->cursor = 0;
    }
}

void input_cursor_end(input_field_t *field)
{
    if (field != NULL) {
        field->cursor = field->len;
    }
}
