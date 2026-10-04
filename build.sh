#!/bin/sh
#
# build.sh
# (c)Noah Wooten 2023 - 2026, All Rights Reserved
#
# Build an application with the Bolt SDK. The same as build.cmd, for Debian
# and macOS.
#
#   ./build.sh examples/calc               a Debug build
#   ./build.sh examples/calc --release     optimised
#   ./build.sh examples/calc --clean       the output thrown away first
#
# One line over tools/bxbuild.py. clang and lld are looked for in
# toolchain/bin first, then on PATH; see docs/setup.md.
#

if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "build: no python found; see docs/setup.md" >&2
    exit 1
fi

if [ -z "$1" ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "usage: ./build.sh <project> [--release] [--clean]"
    exit 0
fi

exec "$PYTHON" "$(dirname "$0")/tools/bxbuild.py" "$@"
