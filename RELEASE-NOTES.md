# vlsx v0.1.2

`vlsx` is a keyboard-driven network toolkit written in pure C99 with zero runtime dependencies beyond libc and a POSIX terminal. Its UI-independent core handles IPv4/IPv6 subnetting, VLSM planning, MAC analysis, SLA metrics, and DHCP timers.

## What's new in v0.1.2

- **Accurate TUI version** — the titlebar now reports the version passed to `make VERSION=...` instead of the hardcoded `v0.1`.
- **Release pipeline** — every release now dispatches an automatic rebuild to the package repository, and Termux packages use clean `vlsx` package names while keeping the `vlsx-termux_` filename prefix.
- **Installation guide** — Homebrew, native Linux packages, Termux, and prebuilt archives are now documented in one place.
- **Homebrew tap** — the README links directly to `b1sted/homebrew-tap`, where release CI maintains the formula.
- **Release verification** — checksum, GPG, and GitHub attestation commands are documented for every artifact.
- **Core toolkit** — IPv4/VLSM/IPv6 calculators with subnet details and allocation plans, MAC conversion and multicast mapping, availability/goodput/cost metrics, and DHCPv4 timers.
- **Zero-dependency binary** — a keyboard-driven TUI for any POSIX terminal, with the termbox2 backend bundled into the build.

## Distribution

Homebrew (`brew install b1sted/tap/vlsx`), `.deb`/`.rpm`/Arch packages, native Termux packages, macOS ARM and Intel archives, and Linux builds for amd64, arm64, armv7, riscv64, and Luckfox ARM.

## Verification

```bash
sha256sum -c SHA256SUMS.txt --ignore-missing
curl -fsSL https://github.com/b1sted.gpg | gpg --import && gpg --verify SHA256SUMS.txt.asc SHA256SUMS.txt
gh attestation verify <file> --repo b1sted/vlsx
```
