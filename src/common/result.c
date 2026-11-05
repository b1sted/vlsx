/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "common/result.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void calc_status_clear(calc_status_t *status)
{
    if (status == NULL) {
        return;
    }

    memset(status, 0, sizeof(*status));
    status->code = CALC_OK;
}

void calc_status_fail(calc_status_t *status, calc_error_t code, const char *message)
{
    if (status == NULL) {
        return;
    }

    status->ok = false;
    status->code = code;
    status->message[0] = '\0';

    if (message != NULL) {
        snprintf(status->message, sizeof(status->message), "%s", message);
    }
}

void calc_status_failf(calc_status_t *status, calc_error_t code, const char *format, ...)
{
    va_list args;

    if (status == NULL) {
        return;
    }

    status->ok = false;
    status->code = code;
    status->message[0] = '\0';

    if (format == NULL) {
        return;
    }

    va_start(args, format);
    vsnprintf(status->message, sizeof(status->message), format, args);
    va_end(args);
}
