#!/usr/bin/env bash
#
# run.sh
# (c)Noah Wooten 2023 - 2026, All Rights Reserved
#
# Put applications on the SDK's disk and start the machine, in a window. The
# same as run.cmd, for Debian and macOS.
#
#   ./run.sh                            the machine, as its disk was left
#   ./run.sh examples/calc              a project's newest build on it first
#   ./run.sh examples/calc/out/debug/sdkcalc.bxf    an application by itself
#   ./run.sh examples/calc/out/debug/sdkcalc.bxi    a package, to install
#   ./run.sh --clean examples/calc      a fresh disk before anything goes on
#   ./run.sh --memory 1024              anything else goes to run_qemu.py
#
# An argument that names a file or a folder is something to put on the disk,
# and anything else is passed to run_qemu.py. The disk is run/bolt-disk.img,
# made from image/fs.bin the first time and kept until --clean. QEMU has to be
# installed; see docs/setup.md.
#

HERE="$(cd "$(dirname "$0")" && pwd)"

if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    echo "run: no python found; see docs/setup.md" >&2
    exit 1
fi

CLEAN=()
WHAT=()
EXTRA=()

for arg in "$@"; do
    case "$arg" in
        --clean)
            CLEAN=(--clean) ;;
        --help|-h)
            sed -n '2,19p' "$0" | sed 's/^# \{0,1\}//'
            exit 0 ;;
        *)
            if [ -e "$arg" ]; then
                WHAT+=("$arg")
            else
                EXTRA+=("$arg")
            fi ;;
    esac
done

"$PYTHON" "$HERE/tools/slipstream.py" --disk "$HERE/run/bolt-disk.img" \
    --image "$HERE/image/fs.bin" "${CLEAN[@]}" "${WHAT[@]}" || exit 1

# --keep-disk because the disk is this script's to keep: left to itself,
# run_qemu.py would make it again whenever the image was newer.
echo
echo "starting the machine"
exec "$PYTHON" "$HERE/tools/run_qemu.py" --efi "$HERE/image/bootx64.efi" \
    --fs-image "$HERE/image/fs.bin" --run-dir "$HERE/run" --keep-disk \
    --interactive "${EXTRA[@]}"
