<h3 align="center">vlsx</h3>

<p align="center">
  <strong>
    A lightweight terminal toolkit in C for network engineers to calculate VLSM, multicast MACs, SLA metrics, and DHCP timers.
  </strong>
</p>
<p align="center">
  <a href="#license"><img
    src="https://img.shields.io/badge/License-GNU%20GPL%20v3.0-green"
    alt="License"
  /></a>
</p>

## Features

The toolkit is divided into 4 core modules based on fundamental networking theory:

### 1. IP & Subnets (IPv4 / IPv6 Calculator)
*   **Subnetting & VLSM:** Calculate Network Address, Broadcast, and Host ranges instantly. Supports Variable Length Subnet Mask (VLSM) planning.
*   **Base Conversions:** Translate IP addresses and masks between Decimal, Binary, and Hexadecimal formats.
*   **IPv6 Optimization:** Automatically format and compress IPv6 addresses (removing leading zeros and applying `::` rules).

### 2. MAC & Multicast Tools (Layer 2)
*   **Multicast MAC Generator:** Convert any standard IPv4 Multicast address (e.g., `239.1.1.15`) into its corresponding Ethernet Multicast MAC address (e.g., `01-00-5E-...`).
*   **OUI / Hex Operations:** Fast validation and conversion of 48-bit physical addresses.

### 3. SLA & Performance Metrics
*   **Availability Calculator (SLA):** Convert downtime (hours/minutes) into uptime percentages (e.g., 99.99%).
*   **Goodput Estimation:** Calculate effective throughput ($G = T - O$) by subtracting protocol overheads (headers, preambles, interpacket gaps).
*   **CapEx / OpEx:** Basic summation tools for network cost planning.

### 4. Protocol Timers (DHCP)
*   **Lease Time Calculator:** Input a DHCP lease time to instantly calculate renewal timers:
    *   $T_0$ (50%) — Unicast `DHCPREQUEST` timer.
    *   $T_1$ (87.5%) — Broadcast `DHCPREQUEST` timer.

## Build and run

### Compilation from source
Requires any standard C99 compiler and `make`.

```bash
make
```

### Usage

```bash
./bin/vlsx
```

## License

Distributed under the **GNU GPL v3.0**.

You are free to use, modify, distribute, and sell this software. However, any derivative works or modifications must be distributed under the same license, the source code must be made openly available, and you must preserve the copyright notice and license file.

Full text of the license: [LICENSE](./LICENSE).