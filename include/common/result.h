/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_RESULT_H
#define VLSX_COMMON_RESULT_H

#include <stdbool.h>

#include "common/limits.h"

typedef enum {
    CALC_OK = 0,
    CALC_ERR_SYNTAX,
    CALC_ERR_RANGE,
    CALC_ERR_CAPACITY,
    CALC_ERR_ALLOCATION,
} calc_error_t;

typedef struct {
    bool ok;
    calc_error_t code;
    char message[MEDIUM_BUFFER];
} calc_status_t;

void calc_status_clear(calc_status_t *status);
void calc_status_fail(calc_status_t *status, calc_error_t code, const char *message);
void calc_status_failf(calc_status_t *status, calc_error_t code, const char *format, ...);

#endif /* VLSX_COMMON_RESULT_H */
