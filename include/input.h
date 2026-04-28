/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#define MEDIUM_BUFFER 128
#define SMALL_BUFFER 64
#define MINI_BUFFER 32
#define TINY_BUFFER 16
#define HEX_BUFFER 8

#define V4_INPUT_MAX 19

typedef struct {
    char error_msg[MEDIUM_BUFFER];
    char *buf;
    int cap;
    int len;
    int cursor;
} InputField;

void input_insert(InputField *field, char ch, bool (*accept)(char));
void input_backspace(InputField *field);
void input_field_clear(InputField *field);

#endif /* INPUT_H */
