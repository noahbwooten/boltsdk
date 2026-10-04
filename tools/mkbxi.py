"""BoltOS application installer packages.

    python mkbxi.py --ini app.ini --module out/calc.bxf --out out/calc.bxi
    python mkbxi.py --check <package.bxi>

A package is a single file holding an application's modules compressed, the
registry values it wants and the links it should be reachable by; bxiutil is
what reads one, and installing it is opening it.

--ini makes one from an application's project file, which is what the SDK's
bxbuild.py does after a link: the [application] and [package] sections say
what it is called and where its links go, [registry] what it writes down, and
the module is the one just built. The system's own packages are described in
make_fs.py, which calls write() below for each.

The records are BXI_HEADER, BXI_FILE, BXI_REGVALUE and BXI_SHORTCUT in bxi.h.
They are mirrored here rather than parsed out of the header, the same way
mklink.py mirrors applink.h and bdfs3.py mirrors bdfs3.h; a field added there
has to be added below, and the assertions on the record sizes are what catch
forgetting.

--check reads a package back: every count, offset and length is walked, every
member is inflated and its CRC-32 compared. It is the whole of the host test,
and what it proves is that a package this writes is one the format describes.
Whether BoltOS reads it is a separate question and is answered by booting one.
"""

import argparse
import configparser
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

MAGIC = 0x50495842    # 'BXIP'
VERSION = 1

# Magic, version, header size, flags; the six names; the three table pairs;
# payload and installed size; reserved.
HEADER = "<4I32s64s64s32s32s128s6I3Q16I"
HEADER_SIZE = 480

# Name, offsets and lengths, checksum, method, attributes, flags.
FILE = "<128s3Q4I"
FILE_SIZE = 168

# Hive, type, path, name, string value, number, flags, reserved.
REGVALUE = "<2I192s64s256sQ2I"
REGVALUE_SIZE = 536

# Target, name, icon, where and how, the window it describes, reserved.
SHORTCUT = "<128s64s32s2I5I4I"
SHORTCUT_SIZE = 268

assert struct.calcsize(HEADER) == HEADER_SIZE
assert struct.calcsize(FILE) == FILE_SIZE
assert struct.calcsize(REGVALUE) == REGVALUE_SIZE
assert struct.calcsize(SHORTCUT) == SHORTCUT_SIZE

# bxi.h
FLAG_CUSTOMUI = 0x00000001

METHOD_STORE = 0
METHOD_DEFLATE = 1

ACTIONS = 0
DESKTOP = 1

OPTIONAL = 0x00000001
DEFAULT = 0x00000002

# registry.h
HIVE_LOCALMACHINE = 0
HIVE_LOCALCONFIGURATION = 1

REGTYPE_KEY = 0
REGTYPE_STRING = 1
REGTYPE_QWORD = 2
REGTYPE_DWORD = 3
REGTYPE_WORD = 4
REGTYPE_BOOLEAN = 5



def field(text, size):
    """One fixed field, terminated. A value that filled it would run into the
    field after it when BoltOS reads the record back."""
    raw = text.encode("ascii")

    if len(raw) >= size:
        raise ValueError("%r does not fit in %d bytes" % (text, size))

    return raw + (b"\0" * (size - len(raw)))


def pack(data):
    """One member, as it is stored. Raw deflate, which is what ArcInflate
    reads; stored when compressing did not help, so a member is never larger
    packed than it was."""
    packed = zlib.compress(data, 9)[2:-4]

    if len(packed) >= len(data):
        return METHOD_STORE, data

    return METHOD_DEFLATE, packed


def source_path(source, binaries):
    if source.startswith("osroot"):
        return os.path.join(ROOT, source)

    return os.path.join(binaries, source)


def write(package, path, binaries):
    """One package, whole. The tables are laid out before the payload so that
    every offset in them is known by the time the header is written."""
    members = []

    for source, name in package["files"]:
        with open(source_path(source, binaries), "rb") as f:
            data = f.read()

        method, stored = pack(data)
        members.append((name, data, method, stored))

    files = package["files"]
    registry = package.get("registry", [])
    shortcuts = package.get("shortcuts", [])

    file_offset = HEADER_SIZE
    reg_offset = file_offset + (len(files) * FILE_SIZE)
    shortcut_offset = reg_offset + (len(registry) * REGVALUE_SIZE)
    payload_offset = shortcut_offset + (len(shortcuts) * SHORTCUT_SIZE)

    table = b""
    payload = b""
    installed = 0

    for name, data, method, stored in members:
        table += struct.pack(FILE, field(name, 128),
                             len(payload), len(data), len(stored),
                             zlib.crc32(data) & 0xFFFFFFFF, method, 0, 0)

        payload += stored
        installed += len(data)

    for hive, kind, keypath, name, value, number in registry:
        table += struct.pack(REGVALUE, hive, kind,
                             field(keypath, 192), field(name, 64),
                             field(value, 256), number, 0, 0)

    for link in shortcuts:
        table += struct.pack(SHORTCUT, field(link["target"], 128),
                             field(link["name"], 64),
                             field(link.get("icon", ""), 32),
                             link["where"], link.get("flags", 0),
                             link.get("x", 0), link.get("y", 0),
                             link.get("w", 0), link.get("h", 0),
                             link.get("windowflags", 0),
                             0, 0, 0, 0)

    flags = FLAG_CUSTOMUI if package.get("customui") else 0

    header = struct.pack(HEADER, MAGIC, VERSION, HEADER_SIZE, flags,
                         field(package["appid"], 32),
                         field(package["name"], 64),
                         field(package["publisher"], 64),
                         field(package["version"], 32),
                         field(package.get("icon", ""), 32),
                         field(package.get("customui", ""), 128),
                         len(files), file_offset,
                         len(registry), reg_offset,
                         len(shortcuts), shortcut_offset,
                         payload_offset, len(payload), installed,
                         *([0] * 16))

    with open(path, "wb") as out:
        out.write(header)
        out.write(table)
        out.write(payload)

    return HEADER_SIZE + len(table) + len(payload)


def trim(raw):
    """One fixed field, back to a string."""
    return raw.split(b"\0")[0].decode("ascii")


def check(path):
    """A package, read back. Every count, offset and length is walked and every
    member is inflated and checksummed, which is what says the writer above and
    the header describe the same file."""
    with open(path, "rb") as f:
        blob = f.read()

    if len(blob) < HEADER_SIZE:
        raise SystemExit("%s: shorter than a header" % path)

    fields = struct.unpack(HEADER, blob[:HEADER_SIZE])
    magic, version, header_size, flags = fields[0:4]

    if magic != MAGIC:
        raise SystemExit("%s: magic %08X is not BXIP" % (path, magic))

    if version != VERSION or header_size != HEADER_SIZE:
        raise SystemExit("%s: version %d, header %d bytes"
                         % (path, version, header_size))

    appid, name, publisher, appversion, icon, customui = \
        [trim(f) for f in fields[4:10]]

    filecount, fileoffset, regcount, regoffset, cutcount, cutoffset = \
        fields[10:16]
    payload_offset, payload_size, installed = fields[16:19]

    print("  %s" % path)
    print("    %-14s %s (%s %s)" % (appid, name, publisher, appversion))
    print("    %-14s %s%s" % ("icon", icon or "none",
                              ", custom ui " + customui if flags else ""))
    print("    %-14s %d files, %d values, %d shortcuts"
          % ("tables", filecount, regcount, cutcount))
    print("    %-14s %d bytes packed, %d unpacked"
          % ("payload", payload_size, installed))

    if payload_offset + payload_size != len(blob):
        raise SystemExit("%s: payload ends at %d, file is %d bytes"
                         % (path, payload_offset + payload_size, len(blob)))

    total = 0

    for i in range(filecount):
        at = fileoffset + (i * FILE_SIZE)
        member = struct.unpack(FILE, blob[at:at + FILE_SIZE])

        member_name = trim(member[0])
        offset, size, packed, crc, method = member[1:6]

        if offset + packed > payload_size:
            raise SystemExit("%s: %s runs past the payload"
                             % (path, member_name))

        at = payload_offset + offset
        stored = blob[at:at + packed]

        data = stored if method == METHOD_STORE \
            else zlib.decompress(stored, -15)

        if len(data) != size:
            raise SystemExit("%s: %s unpacked to %d, header says %d"
                             % (path, member_name, len(data), size))

        if (zlib.crc32(data) & 0xFFFFFFFF) != crc:
            raise SystemExit("%s: %s checksum" % (path, member_name))

        print("    %-14s %s, %d -> %d bytes, crc %08X"
              % ("member", member_name, size, packed, crc))

        total += size

    if total != installed:
        raise SystemExit("%s: members total %d, header says %d"
                         % (path, total, installed))

    for i in range(regcount):
        at = regoffset + (i * REGVALUE_SIZE)
        value = struct.unpack(REGVALUE, blob[at:at + REGVALUE_SIZE])

        print("    %-14s hive %d type %d %s/%s"
              % ("value", value[0], value[1], trim(value[2]), trim(value[3])))

    for i in range(cutcount):
        at = cutoffset + (i * SHORTCUT_SIZE)
        link = struct.unpack(SHORTCUT, blob[at:at + SHORTCUT_SIZE])

        where = "desktop" if link[3] == DESKTOP else "actions"

        print("    %-14s %s -> %s, %s%s"
              % ("shortcut", trim(link[1]), trim(link[0]), where,
                 ", optional" if link[4] & OPTIONAL else ""))

    return 0


# -- From a project file -------------------------------------------------------

DESKTOP_LINK = {
    "none": None,                    # no desktop link at all
    "optional": OPTIONAL,            # offered, and left unticked
    "default": OPTIONAL | DEFAULT,   # offered, and ticked
    "always": 0,                     # made without asking
}

REGTYPES = {
    "string": REGTYPE_STRING, "qword": REGTYPE_QWORD, "dword": REGTYPE_DWORD,
    "word": REGTYPE_WORD, "boolean": REGTYPE_BOOLEAN,
}


def read_ini(path):
    """A project file, with its keys left as they were written: configparser
    folds them to lower case by default, which would fold the registry paths
    in [registry] with them."""
    parser = configparser.ConfigParser(delimiters=("=",),
                                       interpolation=None)
    parser.optionxform = str

    if not parser.read(path, encoding="utf-8"):
        raise SystemExit("mkbxi: %s could not be read" % path)

    return parser


def package_from_ini(path, module):
    """What one project's package holds, as write() takes it. The module is
    the file just built, installed under the name the project gives it; the
    shortcuts name it the same way, since the installer joins them onto the
    folder it installs into."""
    ini = read_ini(path)
    here = os.path.dirname(os.path.abspath(path))

    application = ini["application"] if ini.has_section("application") else {}
    package = ini["package"] if ini.has_section("package") else {}

    name = application.get("name", "").strip()
    stem = application.get("module", "").strip()
    icon = package.get("icon", application.get("icon", "")).strip()

    if not name or not stem:
        raise SystemExit("mkbxi: %s: [application] needs a name and a module"
                         % path)

    version = application.get("version", "1.0").strip()
    target = stem + ".bxf"

    files = [(os.path.abspath(module), target)]

    # Anything else the application installs beside its module, as
    # host=member pairs. The host path is the project's; the member is where
    # it goes under the install folder.
    for pair in package.get("files", "").split():
        host, _, member = pair.partition("=")

        if not member:
            raise SystemExit("mkbxi: %s: files wants host=member, not %r"
                             % (path, pair))

        files.append((os.path.join(here, host), member))

    registry = []

    if ini.has_section("registry"):
        for key, value in ini["registry"].items():
            keypath, _, valuename = key.rpartition("/")
            kind, _, data = value.partition(":")
            kind = kind.strip().lower()

            if kind not in REGTYPES or not keypath:
                raise SystemExit("mkbxi: %s: [registry] %s = %s is not "
                                 "path/name = type:value" % (path, key, value))

            number = 0
            text = ""

            if kind == "string":
                text = data.strip()
            else:
                number = int(data.strip(), 0)

            registry.append((HIVE_LOCALMACHINE, REGTYPES[kind], keypath,
                             valuename, text, number))

    shortcuts = []

    if package.get("actions", "yes").strip().lower() not in ("no", "false",
                                                             "0"):
        shortcuts.append(dict(target=target, name=name, icon=icon,
                              where=ACTIONS, flags=0))

    desktop = package.get("desktop", "optional").strip().lower()

    if desktop not in DESKTOP_LINK:
        raise SystemExit("mkbxi: %s: desktop is one of %s"
                         % (path, ", ".join(DESKTOP_LINK)))

    if DESKTOP_LINK[desktop] is not None:
        shortcuts.append(dict(target=target, name=name, icon=icon,
                              where=DESKTOP, flags=DESKTOP_LINK[desktop]))

    return dict(appid=package.get("appid", stem).strip(),
                name=name,
                publisher=package.get("publisher", "Unknown").strip(),
                version=package.get("version", version).strip(),
                icon=icon,
                files=files,
                registry=registry,
                shortcuts=shortcuts)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--check", metavar="PACKAGE",
                        help="read a package back rather than writing one")
    parser.add_argument("--ini", help="the project file to package")
    parser.add_argument("--module", help="the module the project built")
    parser.add_argument("--out", help="the package to write")
    args = parser.parse_args(argv)

    if args.check:
        return check(args.check)

    if not (args.ini and args.module and args.out):
        parser.error("--ini, --module and --out together, or --check")

    package = package_from_ini(args.ini, args.module)
    directory = os.path.dirname(os.path.abspath(args.out))

    if not os.path.isdir(directory):
        os.makedirs(directory)

    size = write(package, args.out, directory)
    print("  %-44s %d bytes" % (args.out, size))
    return 0


if __name__ == "__main__":
    sys.exit(main())
