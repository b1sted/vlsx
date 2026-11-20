/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>

#include "common/result.h"
#include "core/net/ipv4/ipv4_calc.h"
#include "core/net/ipv4/ipv4_class.h"
#include "core/net/ipv4/ipv4_validate.h"
#include "test_runner.h"

void test_ipv4(void);

static void test_ipv4_valid_cidrs(void);
static void test_ipv4_boundaries(void);
static void test_ipv4_errors(void);
static void test_ipv4_classes(void);

static void test_ipv4_valid_cidrs(void)
{
    ipv4_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv4_calculate("192.168.0.54/26", &result, &status));
    TEST_CHECK(result.base.octets[0] == 192);
    TEST_CHECK(result.base.octets[1] == 168);
    TEST_CHECK(result.base.octets[2] == 0);
    TEST_CHECK(result.base.octets[3] == 54);
    TEST_CHECK(result.prefix == 26);
    TEST_CHECK(result.network.octets[0] == 192);
    TEST_CHECK(result.network.octets[1] == 168);
    TEST_CHECK(result.network.octets[2] == 0);
    TEST_CHECK(result.network.octets[3] == 0);
    TEST_CHECK(result.broadcast.octets[3] == 63);
    TEST_CHECK(result.first_host.octets[3] == 1);
    TEST_CHECK(result.last_host.octets[3] == 62);
    TEST_CHECK(result.host_count == 62);
    TEST_CHECK(result.class == IPV4_CLASS_PRIVATE_C);

    TEST_CHECK(ipv4_calculate("10.0.0.0/8", &result, &status));
    TEST_CHECK(result.prefix == 8);
    TEST_CHECK(result.host_count == UINT32_C(16777214));
    TEST_CHECK(result.class == IPV4_CLASS_PRIVATE_A);

    TEST_CHECK(ipv4_calculate("172.16.0.0/12", &result, &status));
    TEST_CHECK(result.host_count == UINT32_C(1048574));
    TEST_CHECK(result.class == IPV4_CLASS_PRIVATE_B);
}

static void test_ipv4_boundaries(void)
{
    ipv4_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv4_calculate("192.168.0.0/31", &result, &status));
    TEST_CHECK(result.host_count == 2);
    TEST_CHECK(result.first_host.octets[3] == 0);
    TEST_CHECK(result.last_host.octets[3] == 1);

    TEST_CHECK(ipv4_calculate("192.168.0.1/31", &result, &status));
    TEST_CHECK(result.network.octets[3] == 0);
    TEST_CHECK(result.first_host.octets[3] == 0);
    TEST_CHECK(result.last_host.octets[3] == 1);

    TEST_CHECK(ipv4_calculate("192.168.0.5/32", &result, &status));
    TEST_CHECK(result.host_count == 1);
    TEST_CHECK(result.first_host.octets[3] == 5);
    TEST_CHECK(result.last_host.octets[3] == 5);

    TEST_CHECK(ipv4_calculate("0.0.0.0/0", &result, &status));
    TEST_CHECK(result.host_count == UINT32_C(4294967294));
    TEST_CHECK(result.class == IPV4_CLASS_DEFAULT_ROUTE);
}

static void test_ipv4_errors(void)
{
    ipv4_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!ipv4_calculate("192.168.0.256/24", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!ipv4_calculate("192.168.0.0/33", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);

    TEST_CHECK(!ipv4_calculate("192.168.0.0", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!ipv4_calculate("", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);

    TEST_CHECK(!ipv4_calculate("1.2.3.4/24x", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

static void test_ipv4_classes(void)
{
    ipv4_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv4_calculate("127.0.0.1/8", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_LOOPBACK);

    TEST_CHECK(ipv4_calculate("169.254.1.1/16", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_APIPA);

    TEST_CHECK(ipv4_calculate("100.64.0.1/10", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_CGNAT);

    TEST_CHECK(ipv4_calculate("239.1.1.15/4", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_MULTICAST);

    TEST_CHECK(ipv4_calculate("192.0.2.1/24", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_TEST_NET_1);

    TEST_CHECK(ipv4_calculate("198.51.100.1/24", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_TEST_NET_2);

    TEST_CHECK(ipv4_calculate("203.0.113.1/24", &result, &status));
    TEST_CHECK(result.class == IPV4_CLASS_TEST_NET_3);
}

void test_ipv4(void)
{
    test_ipv4_valid_cidrs();
    test_ipv4_boundaries();
    test_ipv4_errors();
    test_ipv4_classes();
}
