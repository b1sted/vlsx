/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_SLA_SLA_CALC_H
#define VLSX_CORE_NET_SLA_SLA_CALC_H

#include <stdbool.h>

#include "common/result.h"

typedef struct {
    double availability_percent;
    double allowed_downtime_9999_minutes;
    double allowed_downtime_99999_minutes;
} sla_availability_result_t;

typedef struct {
    double nominal_mbps;
    double overhead_mbps;
    double goodput_mbps;
} sla_goodput_result_t;

typedef struct {
    double capex;
    double opex;
    double total;
} sla_costs_result_t;

bool sla_validate_number(const char *text);
bool sla_accept_number(char ch);
bool sla_validate_cost_list(const char *text);
bool sla_accept_cost_list(char ch);

bool sla_availability_calculate(const char *uptime_hours, const char *downtime_hours,
                                sla_availability_result_t *out, calc_status_t *status);
bool sla_goodput_calculate(const char *bandwidth_mbps, const char *overhead_percent,
                           sla_goodput_result_t *out, calc_status_t *status);
bool sla_costs_calculate(const char *capex_text, const char *opex_text, sla_costs_result_t *out,
                         calc_status_t *status);

#endif /* VLSX_CORE_NET_SLA_SLA_CALC_H */
