/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/dhcp/dhcp_calc.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool dhcp_parse_lease(const char *text, uint32_t *out);

static bool dhcp_parse_lease(const char *text, uint32_t *out)
{
    char *end = NULL;
    uint64_t value = 0;

    if (text == NULL || out == NULL || text[0] == '\0') {
        return false;
    }

    errno = 0;
    value = (uint64_t)strtoull(text, &end, 10);

    if (end == text || end == NULL || *end != '\0' || errno == ERANGE || value == 0 ||
        value > (uint64_t)UINT32_MAX) {
        return false;
    }

    *out = (uint32_t)value;
    return true;
}

bool dhcp_validate_lease(const char *text)
{
    uint32_t value = 0;

    return dhcp_parse_lease(text, &value);
}

bool dhcp_accept_lease(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool dhcp_timers_calculate(const char *lease_text, dhcp_timer_result_t *out, calc_status_t *status)
{
    uint32_t lease = 0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!dhcp_parse_lease(lease_text, &lease)) {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid lease time. Use positive minutes.");
        return false;
    }

    out->lease_minutes = lease;
    out->t0_minutes = (double)lease * 0.5;
    out->t1_minutes = (double)lease * 0.875;
    out->t2_minutes = (double)lease;

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
