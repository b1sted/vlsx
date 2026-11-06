/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "core/net/sla/sla_calc.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static bool sla_parse_number(const char *text, double *out);
static bool sla_parse_cost_list(const char *text, double *out);

static bool sla_parse_number(const char *text, double *out)
{
    char *end = NULL;
    double value = 0.0;

    if (text == NULL || out == NULL || text[0] == '\0') {
        return false;
    }

    errno = 0;
    value = strtod(text, &end);

    if (end == text || end == NULL || *end != '\0' || errno == ERANGE || value < 0.0) {
        return false;
    }

    *out = value;
    return true;
}

static bool sla_parse_cost_list(const char *text, double *out)
{
    const char *cursor = text;
    double total = 0.0;
    bool has_value = false;

    if (text == NULL || out == NULL || text[0] == '\0') {
        return false;
    }

    while (*cursor != '\0') {
        while (*cursor == ' ') {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        char *end = NULL;
        errno = 0;
        const double value = strtod(cursor, &end);

        if (end == cursor || end == NULL || errno == ERANGE || value < 0.0) {
            return false;
        }

        total += value;
        has_value = true;
        cursor = end;

        while (*cursor == ' ') {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        if (*cursor != ',') {
            return false;
        }

        cursor++;

        if (*cursor == '\0') {
            return false;
        }
    }

    if (!has_value) {
        return false;
    }

    *out = total;
    return true;
}

bool sla_validate_number(const char *text)
{
    double value = 0.0;

    return sla_parse_number(text, &value);
}

bool sla_accept_number(char ch)
{
    return (ch >= '0' && ch <= '9') || ch == '.';
}

bool sla_validate_cost_list(const char *text)
{
    double value = 0.0;

    return sla_parse_cost_list(text, &value);
}

bool sla_accept_cost_list(char ch)
{
    return (ch >= '0' && ch <= '9') || ch == '.' || ch == ',' || ch == ' ';
}

bool sla_availability_calculate(const char *uptime_hours, const char *downtime_hours,
                                sla_availability_result_t *out, calc_status_t *status)
{
    double total = 0.0;
    double downtime = 0.0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!sla_parse_number(uptime_hours, &total) || !sla_parse_number(downtime_hours, &downtime)) {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid numeric input.");
        return false;
    }

    if (total <= 0.0) {
        calc_status_fail(status, CALC_ERR_RANGE, "Uptime base must be greater than zero.");
        return false;
    }

    if (downtime > total) {
        calc_status_fail(status, CALC_ERR_RANGE, "Downtime cannot exceed the uptime base.");
        return false;
    }

    out->availability_percent = ((total - downtime) / total) * 100.0;
    out->allowed_downtime_9999_minutes = total * 0.0001 * 60.0;
    out->allowed_downtime_99999_minutes = total * 0.00001 * 60.0;

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}

bool sla_goodput_calculate(const char *bandwidth_mbps, const char *overhead_percent,
                           sla_goodput_result_t *out, calc_status_t *status)
{
    double bandwidth = 0.0;
    double overhead = 0.0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!sla_parse_number(bandwidth_mbps, &bandwidth) ||
        !sla_parse_number(overhead_percent, &overhead)) {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid numeric input.");
        return false;
    }

    if (bandwidth <= 0.0) {
        calc_status_fail(status, CALC_ERR_RANGE, "Bandwidth must be greater than zero.");
        return false;
    }

    if (overhead > 100.0) {
        calc_status_fail(status, CALC_ERR_RANGE, "Overhead cannot exceed 100 percent.");
        return false;
    }

    out->nominal_mbps = bandwidth;
    out->overhead_mbps = bandwidth * (overhead / 100.0);
    out->goodput_mbps = out->nominal_mbps - out->overhead_mbps;

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}

bool sla_costs_calculate(const char *capex_text, const char *opex_text, sla_costs_result_t *out,
                         calc_status_t *status)
{
    double capex = 0.0;
    double opex = 0.0;

    if (out == NULL || status == NULL) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    calc_status_clear(status);

    if (!sla_parse_cost_list(capex_text, &capex) || !sla_parse_cost_list(opex_text, &opex)) {
        calc_status_fail(status, CALC_ERR_SYNTAX, "Invalid cost list. Use positive numbers.");
        return false;
    }

    out->capex = capex;
    out->opex = opex;
    out->total = capex + opex;

    status->ok = true;
    status->code = CALC_OK;
    status->message[0] = '\0';

    return true;
}
