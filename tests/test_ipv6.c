/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/result.h"
#include "core/net/ipv6/ipv6_calc.h"
#include "core/net/ipv6/ipv6_class.h"
#include "core/net/ipv6/ipv6_validate.h"
#include "test_runner.h"

void test_ipv6(void);

static void test_ipv6_target_example(void);
static void test_ipv6_representations(void);
static void test_ipv6_network_calculation(void);
static void test_ipv6_types(void);
static void test_ipv6_ipv4_embedded(void);
static void test_ipv6_errors(void);

static void test_ipv6_target_example(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv6_calculate("fe80:0000:0000:0000:0a00:0027:fea8:3ebd/64", &result, &status));
    TEST_CHECK(result.prefix == 64);
    TEST_CHECK(strcmp(result.original, "fe80:0000:0000:0000:0a00:0027:fea8:3ebd") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "fe80:0:0:0:a00:27:fea8:3ebd") == 0);
    TEST_CHECK(strcmp(result.compressed, "fe80::a00:27:fea8:3ebd") == 0);
    TEST_CHECK(strcmp(result.expanded, "fe80:0000:0000:0000:0a00:0027:fea8:3ebd") == 0);
    TEST_CHECK(strcmp(result.network_text, "fe80::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0a00:0027:fea8:3ebd") == 0);
    TEST_CHECK(result.type == IPV6_TYPE_LINK_LOCAL);
    TEST_CHECK(strcmp(ipv6_type_name(result.type), "Link-Local") == 0);
    TEST_CHECK(result.network.hextets[0] == UINT16_C(0xFE80));

    for (size_t i = 1; i < IPV6_HEXTET_COUNT; i++) {
        TEST_CHECK(result.network.hextets[i] == 0);
    }

    TEST_CHECK(result.interface_id.hextets[0] == 0);
    TEST_CHECK(result.interface_id.hextets[1] == 0);
    TEST_CHECK(result.interface_id.hextets[2] == 0);
    TEST_CHECK(result.interface_id.hextets[3] == 0);
    TEST_CHECK(result.interface_id.hextets[4] == UINT16_C(0x0A00));
    TEST_CHECK(result.interface_id.hextets[5] == UINT16_C(0x0027));
    TEST_CHECK(result.interface_id.hextets[6] == UINT16_C(0xFEA8));
    TEST_CHECK(result.interface_id.hextets[7] == UINT16_C(0x3EBD));
}

static void test_ipv6_representations(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv6_calculate("2001:0db8:0000:0000:0000:0000:0000:0001/64", &result, &status));
    TEST_CHECK(strcmp(result.compressed, "2001:db8::1") == 0);
    TEST_CHECK(strcmp(result.network_text, "2001:db8::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0000:0000:0000:0001") == 0);

    TEST_CHECK(ipv6_calculate("2001:0:0:1:0:0:1:1/128", &result, &status));
    TEST_CHECK(strcmp(result.compressed, "2001::1:0:0:1:1") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "2001:0:0:1:0:0:1:1") == 0);
    TEST_CHECK(strcmp(result.expanded, "2001:0000:0000:0001:0000:0000:0001:0001") == 0);
    TEST_CHECK(result.interface_id_text[0] == '\0');

    TEST_CHECK(ipv6_calculate("1:2:3:4:5:6:7:8/64", &result, &status));
    TEST_CHECK(strcmp(result.compressed, "1:2:3:4:5:6:7:8") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "1:2:3:4:5:6:7:8") == 0);
    TEST_CHECK(strcmp(result.expanded, "0001:0002:0003:0004:0005:0006:0007:0008") == 0);

    TEST_CHECK(ipv6_calculate("::/0", &result, &status));
    TEST_CHECK(strcmp(result.compressed, "::") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "0:0:0:0:0:0:0:0") == 0);
    TEST_CHECK(strcmp(result.expanded, "0000:0000:0000:0000:0000:0000:0000:0000") == 0);
    TEST_CHECK(strcmp(result.network_text, "::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0000:0000:0000:0000:0000:0000:0000:0000") == 0);

    TEST_CHECK(ipv6_calculate("FE80::000A/64", &result, &status));
    TEST_CHECK(strcmp(result.original, "FE80::000A") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "fe80:0:0:0:0:0:0:a") == 0);
    TEST_CHECK(strcmp(result.compressed, "fe80::a") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8::1", &result, &status));
    TEST_CHECK(result.prefix == 64);
}

static void test_ipv6_network_calculation(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv6_calculate("8000::1/1", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "8000::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0000:0000:0000:0000:0000:0000:0000:0001") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:5678:9abc:def0:1234/48", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "1234:5678:9abc:def0:1234") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:5678:9abc:def0:1234/65", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd:1234::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "5678:9abc:def0:1234") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:f678:9abc:def0:1234/65", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd:1234:8000::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "7678:9abc:def0:1234") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:f678:9abc:def0:1234/68", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd:1234:f000::") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0678:9abc:def0:1234") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:5678:9abc:def0:1234/127", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd:1234:5678:9abc:def0:1234") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0000") == 0);

    TEST_CHECK(ipv6_calculate("2001:db8:abcd:1234:5678:9abc:def0:1234/128", &result, &status));
    TEST_CHECK(strcmp(result.network_text, "2001:db8:abcd:1234:5678:9abc:def0:1234") == 0);
    TEST_CHECK(result.interface_id_text[0] == '\0');
}

static void test_ipv6_types(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv6_calculate("::/128", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_UNSPECIFIED);

    TEST_CHECK(ipv6_calculate("::1/128", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_LOOPBACK);
    TEST_CHECK(strcmp(result.network_text, "::1") == 0);

    TEST_CHECK(ipv6_calculate("ff02::1/8", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_MULTICAST);
    TEST_CHECK(strcmp(result.network_text, "ff00::") == 0);

    TEST_CHECK(ipv6_calculate("fd12:3456::1/8", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_UNIQUE_LOCAL);
    TEST_CHECK(strcmp(result.network_text, "fd00::") == 0);

    TEST_CHECK(ipv6_calculate("fec0::1/10", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_SITE_LOCAL);

    TEST_CHECK(ipv6_calculate("2001:db8::1/32", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_DOCUMENTATION);

    TEST_CHECK(ipv6_calculate("2002:0102:0304::1/16", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_6TO4);

    TEST_CHECK(ipv6_calculate("2001:0000::1/32", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_TEREDO);

    TEST_CHECK(ipv6_calculate("2606:4700:4700::1111/32", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_GLOBAL_UNICAST);

    TEST_CHECK(ipv6_calculate("4000::1/16", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_RESERVED);
}

static void test_ipv6_ipv4_embedded(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(ipv6_calculate("::ffff:192.168.0.1/120", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_IPV4_MAPPED);
    TEST_CHECK(strcmp(result.original, "::ffff:192.168.0.1") == 0);
    TEST_CHECK(strcmp(result.no_leading_zeros, "0:0:0:0:0:ffff:c0a8:1") == 0);
    TEST_CHECK(strcmp(result.compressed, "::ffff:c0a8:1") == 0);
    TEST_CHECK(strcmp(result.expanded, "0000:0000:0000:0000:0000:ffff:c0a8:0001") == 0);
    TEST_CHECK(strcmp(result.network_text, "::ffff:c0a8:0") == 0);
    TEST_CHECK(strcmp(result.interface_id_text, "0001") == 0);

    TEST_CHECK(ipv6_calculate("1:2:3:4:5:6:192.0.2.1/128", &result, &status));
    TEST_CHECK(strcmp(result.compressed, "1:2:3:4:5:6:c000:201") == 0);

    TEST_CHECK(ipv6_calculate("::192.0.2.1/96", &result, &status));
    TEST_CHECK(result.type == IPV6_TYPE_IPV4_COMPATIBLE);
}

static void test_ipv6_errors(void)
{
    ipv6_result_t result = {0};
    calc_status_t status = {0};

    TEST_CHECK(!ipv6_validate(NULL));
    TEST_CHECK(!ipv6_validate(""));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:6:7"));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:6:7:8:9"));
    TEST_CHECK(!ipv6_validate("1::2::3"));
    TEST_CHECK(!ipv6_validate(":::"));
    TEST_CHECK(!ipv6_validate("12345::"));
    TEST_CHECK(!ipv6_validate("gggg::"));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:6:7:8/"));
    TEST_CHECK(!ipv6_validate("/64"));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:6:7:8/129"));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:6:7:8/12x"));
    TEST_CHECK(!ipv6_validate("::ffff:192.168.0.256"));
    TEST_CHECK(!ipv6_validate("::ffff:192.168.0.1.2"));
    TEST_CHECK(!ipv6_validate("192.168.0.1"));
    TEST_CHECK(!ipv6_validate(":1::"));
    TEST_CHECK(!ipv6_validate("1::2:"));
    TEST_CHECK(!ipv6_validate("1:2:3:4:5:192.168.0.1"));
    TEST_CHECK(!ipv6_validate("2001:db8::1/64/64"));

    TEST_CHECK(!ipv6_calculate("1:2:3:4:5:6:7:8/129", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_RANGE);
    TEST_CHECK(!ipv6_calculate("gggg::/64", &result, &status));
    TEST_CHECK(status.code == CALC_ERR_SYNTAX);
}

void test_ipv6(void)
{
    test_ipv6_target_example();
    test_ipv6_representations();
    test_ipv6_network_calculation();
    test_ipv6_types();
    test_ipv6_ipv4_embedded();
    test_ipv6_errors();
}
