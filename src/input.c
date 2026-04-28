/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <string.h>

#include "input.h"

void input_insert(InputField *field, char ch, bool (*accept)(char)) {
    if (!accept(ch))
        return;
    if (field->len + 1 >= field->cap)
        return;

    memmove(&field->buf[field->cursor + 1], &field->buf[field->cursor],
            field->len - field->cursor);
    field->buf[field->cursor] = ch;
    field->len++;
    field->cursor++;
    field->buf[field->len] = '\0';
}

void input_backspace(InputField *field) {
    if (!field->cursor)
        return;

    memmove(&field->buf[field->cursor - 1], &field->buf[field->cursor],
            field->len - field->cursor);
    field->len--;
    field->cursor--;
    field->buf[field->len] = '\0';
}

void input_field_clear(InputField *field) {
    if (!field)
        return;

    field->error_msg[0] = '\0';

    if (field->buf)
        field->buf[0] = '\0';

    field->cursor = 0;
    field->len = 0;
}
