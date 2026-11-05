/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV4_CLASS_H
#define VLSX_CORE_NET_IPV4_CLASS_H

#include <stdint.h>

#include "core/net/ipv4/ipv4_types.h"

ipv4_class_t ipv4_classify(const ipv4_addr_t *addr, uint8_t prefix);
const char *ipv4_class_name(ipv4_class_t cls);

#endif /* VLSX_CORE_NET_IPV4_CLASS_H */
