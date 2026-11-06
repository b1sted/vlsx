/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "common/result.h"
#include "core/net/sla/sla_calc.h"
#include "test_runner.h"

void test_metrics(void);

static bool close_to(double first, double second, double tolerance);

static bool close_to(double first, double second, double tolerance)
{
    double difference = first - second;

    if (difference < 0.0) {
        difference = -difference;
    }

    return difference <= tolerance;
}

static void test_availability(void)
{
    sla_availability_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(sla_availability_calculate("8760", "2", &result, &status));
    TEST_CHECK(status.code == CALC_OK);
    TEST_CHECK(close_to(result.availability_percent, 99.9771689498, 0.0000001));
    TEST_CHECK(close_to(result.allowed_downtime_9999_minutes, 52.56, 0.000001));
    TEST_CHECK(close_to(result.allowed_downtime_99999_minutes, 5.256, 0.000001));

    TEST_CHECK(!sla_availability_calculate("0", "0", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!sla_availability_calculate("8760", "9000", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!sla_availability_calculate("invalid", "2", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

static void test_goodput(void)
{
    sla_goodput_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(sla_goodput_calculate("100", "15", &result, &status));
    TEST_CHECK(status.code == CALC_OK);
    TEST_CHECK(close_to(result.nominal_mbps, 100.0, 0.000001));
    TEST_CHECK(close_to(result.overhead_mbps, 15.0, 0.000001));
    TEST_CHECK(close_to(result.goodput_mbps, 85.0, 0.000001));

    TEST_CHECK(!sla_goodput_calculate("0", "15", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!sla_goodput_calculate("100", "101", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!sla_goodput_calculate("100", "wrong", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

static void test_costs(void)
{
    sla_costs_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(sla_costs_calculate("5000, 200, 1000", "100, 50, 20", &result, &status));
    TEST_CHECK(status.code == CALC_OK);
    TEST_CHECK(close_to(result.capex, 6200.0, 0.000001));
    TEST_CHECK(close_to(result.opex, 170.0, 0.000001));
    TEST_CHECK(close_to(result.total, 6370.0, 0.000001));

    TEST_CHECK(sla_costs_calculate(" 12.5 , 7.5 ", "1", &result, &status));
    TEST_CHECK(close_to(result.capex, 20.0, 0.000001));
    TEST_CHECK(close_to(result.opex, 1.0, 0.000001));

    TEST_CHECK(!sla_costs_calculate("1,,2", "3", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!sla_costs_calculate("1,", "3", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!sla_costs_calculate("", "3", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!sla_costs_calculate("-1", "3", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

void test_metrics(void)
{
    test_availability();
    test_goodput();
    test_costs();
}
