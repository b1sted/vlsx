/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV6_CLASS_H
#define VLSX_CORE_NET_IPV6_CLASS_H

#include "core/net/ipv6/ipv6_types.h"

ipv6_type_t ipv6_classify(const ipv6_addr_t *addr);
const char *ipv6_type_name(ipv6_type_t type);

#endif /* VLSX_CORE_NET_IPV6_CLASS_H */
