/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_CORE_NET_IPV4_VALIDATE_H
#define VLSX_CORE_NET_IPV4_VALIDATE_H

#include <stdbool.h>

bool ipv4_validate(const char *input);
bool ipv4_accept(char ch);

#endif /* VLSX_CORE_NET_IPV4_VALIDATE_H */
