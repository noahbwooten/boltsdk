"""BoltOS links.

    python mklink.py <out.bal> --name "Calculator" --target /sys/apps/calc.bxf
                     [--icon AppIcon_Calculator]

Writes one .bal, which is what the Actions menu, the desktop and the startup
folder are made of: a link names a file, what it is called where it is shown,
and the icon it is drawn with. Opening one opens what it names.

The record is APPLINK in applink.h. It is mirrored here rather than parsed out
of the header, the same way bdfs3.py mirrors bdfs3.h; a field added there has
to be added below, and the assertion on the record size is what catches
forgetting.

mkactions.py writes the system's own menu with this, and the SDK ships it for
an application's.
"""

import argparse
import os
import struct
import sys

MAGIC = 0x4B4C4142    # 'BALK'
VERSION = 1

# Magic, version, name, target, icon, arguments, geometry, flags, reserved.
LAYOUT = "<II64s128s32s128s5II8I"
RECORD = 416

assert struct.calcsize(LAYOUT) == RECORD

SUFFIX = ".bal"


def field(text, size):
    """One fixed field, terminated. A value that fills it would run into the
    field after it when the shell reads the record back."""
    raw = text.encode("ascii")

    if len(raw) >= size:
        raise ValueError("%r does not fit in %d bytes" % (text, size))

    return raw + (b"\0" * (size - len(raw)))


def record(name, target, icon=""):
    """One link's bytes. The geometry is left at zero: a target that opens its
    own window is let do so, and a zero size is what tells the shell that."""
    return struct.pack(LAYOUT, MAGIC, VERSION,
                       field(name, 64), field(target, 128),
                       field(icon, 32), field("", 128),
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0)


def write_link(path, name, target, icon=""):
    """One link, written to a host file. Answers how long it is."""
    data = record(name, target, icon)

    with open(path, "wb") as out:
        out.write(data)

    return len(data)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("out", help="the .bal to write")
    parser.add_argument("--name", required=True,
                        help="what the link is called where it is shown")
    parser.add_argument("--target", required=True,
                        help="the path on the volume it opens")
    parser.add_argument("--icon", default="",
                        help="an image the target carries, by name")
    args = parser.parse_args(argv)

    directory = os.path.dirname(os.path.abspath(args.out))

    if not os.path.isdir(directory):
        os.makedirs(directory)

    size = write_link(args.out, args.name, args.target, args.icon)
    print("  %s, %d bytes" % (args.out, size))
    return 0


if __name__ == "__main__":
    sys.exit(main())
