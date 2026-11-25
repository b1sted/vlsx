#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Build, smoke test, and package a release as a Termux .deb.
# Intended to run inside termux/termux-docker from the repository root:
#
#   /entrypoint.sh bash /workspace/scripts/termux-build.sh v1.2.3
#
# Required environment:
#   PKG_NAME      package and installed binary name
#   BIN_PATH      path to the built binary, relative to the repository root
#   HOMEPAGE      package homepage
#   DESCRIPTION   one-line package description
#   MAINTAINER    package maintainer, e.g. "Name <mail@example.com>"
#
# Optional environment:
#   VERSION_FLAG  flag that prints the version (default: -V)
#   ARTIFACT_DIR  artifact output directory (default: artifact)

set -eu

version=${1:-}
version=${version#v}
if [ -z "$version" ]; then
    echo "usage: $0 <version>" >&2
    exit 1
fi

: "${PKG_NAME:?PKG_NAME is required}"
: "${BIN_PATH:?BIN_PATH is required}"
: "${HOMEPAGE:?HOMEPAGE is required}"
: "${DESCRIPTION:?DESCRIPTION is required}"
: "${MAINTAINER:?MAINTAINER is required}"

version_flag=${VERSION_FLAG:--V}
artifact_dir=${ARTIFACT_DIR:-artifact}

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y clang make

make CC=clang VERSION="$version"
"$BIN_PATH" "$version_flag"

arch=$(dpkg --print-architecture)
pkg="${PKG_NAME}-termux_${version}_${arch}.deb"
pkgroot=pkgroot

rm -rf "$pkgroot"
mkdir -p "$pkgroot/DEBIAN" "$pkgroot/data/data/com.termux/files/usr/bin"
cp "$BIN_PATH" "$pkgroot/data/data/com.termux/files/usr/bin/$PKG_NAME"
chmod 0755 "$pkgroot/data/data/com.termux/files/usr/bin/$PKG_NAME"

printf '%s\n' \
    "Package: $PKG_NAME" \
    "Version: $version" \
    "Architecture: $arch" \
    "Maintainer: $MAINTAINER" \
    "Homepage: $HOMEPAGE" \
    "Section: utils" \
    "Priority: optional" \
    "Description: $DESCRIPTION" \
    > "$pkgroot/DEBIAN/control"

dpkg-deb --root-owner-group --build "$pkgroot" "$pkg"

if [ "$(dpkg-deb -f "$pkg" Package)" != "$PKG_NAME" ]; then
    echo "termux-build: $pkg has an unexpected package name" >&2
    exit 1
fi

dpkg -i "$pkg"
"$PKG_NAME" "$version_flag"

mkdir -p "$artifact_dir"
cp "$pkg" "$artifact_dir/"
cp "$BIN_PATH" "$artifact_dir/$PKG_NAME"
chmod -R a+rwX "$artifact_dir"

echo "termux-build: packaged $pkg"
