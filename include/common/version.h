/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef VLSX_COMMON_VERSION_H
#define VLSX_COMMON_VERSION_H

#ifndef VLSX_VERSION
#define VLSX_VERSION "dev"
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define VLSX_ARCH "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define VLSX_ARCH "aarch64"
#elif defined(__arm__) || defined(__ARM_ARCH_7A__)
#define VLSX_ARCH "armv7"
#elif defined(__riscv) && defined(__riscv_xlen) && (__riscv_xlen == 64)
#define VLSX_ARCH "riscv64"
#elif defined(__i386__)
#define VLSX_ARCH "i386"
#else
#define VLSX_ARCH "unknown"
#endif

#endif /* VLSX_COMMON_VERSION_H */
