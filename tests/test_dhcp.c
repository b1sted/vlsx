/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>

#include "common/result.h"
#include "core/net/dhcp/dhcp_calc.h"
#include "test_runner.h"

void test_dhcp(void);

static bool close_to(double first, double second, double tolerance);

static bool close_to(double first, double second, double tolerance)
{
    double difference = first - second;

    if (difference < 0.0) {
        difference = -difference;
    }

    return difference <= tolerance;
}

static void test_dhcp_timers(void)
{
    dhcp_timer_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(dhcp_timers_calculate("600", &result, &status));
    TEST_CHECK(status.code == CALC_OK);
    TEST_CHECK(result.lease_minutes == 600);
    TEST_CHECK(close_to(result.t0_minutes, 300.0, 0.000001));
    TEST_CHECK(close_to(result.t1_minutes, 525.0, 0.000001));
    TEST_CHECK(close_to(result.t2_minutes, 600.0, 0.000001));

    TEST_CHECK(dhcp_timers_calculate("1", &result, &status));
    TEST_CHECK(close_to(result.t0_minutes, 0.5, 0.000001));
    TEST_CHECK(close_to(result.t1_minutes, 0.875, 0.000001));
    TEST_CHECK(close_to(result.t2_minutes, 1.0, 0.000001));
}

static void test_dhcp_errors(void)
{
    dhcp_timer_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!dhcp_timers_calculate("", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!dhcp_timers_calculate("0", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!dhcp_timers_calculate("12.5", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!dhcp_timers_calculate("lease", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!dhcp_timers_calculate("4294967296", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

void test_dhcp(void)
{
    TEST_CHECK(dhcp_validate_lease("600"));
    TEST_CHECK(!dhcp_validate_lease("600.5"));
    TEST_CHECK(!dhcp_validate_lease(""));

    test_dhcp_timers();
    test_dhcp_errors();
}
