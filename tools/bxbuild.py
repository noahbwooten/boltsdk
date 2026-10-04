"""Build a BoltOS application with the Bolt SDK.

    python tools/bxbuild.py examples/calc                 a Debug build
    python tools/bxbuild.py examples/calc --release       optimised
    python tools/bxbuild.py examples/calc --clean         throw the output away first

build.cmd and build.sh in the SDK's folder are this, one line each.

A project is a folder with an app.ini in it, which says everything about the
application that is not C: what it is called, its version, its sources, which
of the system's libraries it calls, the pictures and sounds it carries, and
how it is packaged. docs/guide/project-file.md describes every key.

What comes out, in <project>/out/debug or out/release:

    <module>.bxf    the application, which is what runs
    <module>.bxi    its installer package, when app.ini has a [package]
    <module>.pdb    and .map, for reading a fault address

The compile is the system's own: the flags come from cflags.py, which the
system's build uses too, so an application is built the way the modules it
calls into were. Two things are added. BOLTSDK_APIV, so every Bolt API name
the application writes calls the numbered export that never changes; and
boltapp.h, written from [application] before anything is compiled, so the
version and the names the file carries are the ones app.ini says. Both are
checked in the linked file before anything is attached to it.
"""

import argparse
import concurrent.futures
import contextlib
import io
import json
import os
import re
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SDK = os.path.dirname(HERE)

sys.path.insert(0, HERE)

import cflags
import mkbxi
import multibmpres
import toolchain

INCLUDE = os.path.join(SDK, "include")
LIB = os.path.join(SDK, "lib")

# The libraries an application can link against, which are the system modules
# that export the Bolt API. Named as the modules are, since an import library
# names the module it binds to.
LIBRARIES = ["user", "wndmgr", "cmnctl", "cmnctl2", "psapi", "netapi",
             "boltshell"]

# What the version manifest is, as modres.h lays it out. A field added there is
# added here.
VERSION_SECTION = b".modver"
VERSION_MAGIC = 0x52455642     # 'BVER'
VERSION_SIZE = 312
VERSION_NAME = (248, 64)       # OriginalFilename: offset, field width

# And the SDK record, as boltsdk.h lays it out.
SDK_SECTION = b".boltsdk"
SDK_MAGIC = 0x4B445342         # 'BSDK'

# How long each string may be, from the fields that hold them, less the
# terminator.
LIMITS = dict(name=63, description=63, product=63, copyright=63,
              language=31, icon=31, filename=63)


def fail(message):
    raise SystemExit("bxbuild: " + message)


def api_versions():
    """Every BOLTSDK_APIV_n these headers know, and the latest of them."""
    text = open(os.path.join(INCLUDE, "boltsdk.h"), encoding="utf-8").read()
    known = sorted(int(n) for n in
                   re.findall(r"#define\s+BOLTSDK_APIV_(\d+)\s+\d+", text))

    if not known:
        fail("include/boltsdk.h names no API version")

    return known, known[-1]


# -- The project ----------------------------------------------------------------

class Project(object):
    """One app.ini, read and checked before anything is built from it."""

    def __init__(self, where):
        where = os.path.abspath(where)

        if os.path.isdir(where):
            where = os.path.join(where, "app.ini")

        if not os.path.isfile(where):
            fail("%s: no app.ini" % where)

        self.ini_path = where
        self.root = os.path.dirname(where)
        self.ini = mkbxi.read_ini(where)

        if not self.ini.has_section("application"):
            fail("%s: no [application] section" % where)

        app = self.ini["application"]
        build = self.ini["build"] if self.ini.has_section("build") else {}

        self.name = app.get("name", "").strip()
        self.module = app.get("module", "").strip()

        if not self.name or not self.module:
            fail("%s: [application] needs a name and a module" % where)

        if not re.match(r"^[A-Za-z0-9_\-]+$", self.module):
            fail("module %r: letters, digits, - and _ only, since it is a "
                 "file name on the volume" % self.module)

        self.filename = self.module + ".bxf"
        self.description = app.get("description", self.name).strip()
        self.product = app.get("product", self.name).strip()
        self.copyright = app.get("copyright", "").strip()
        self.language = app.get("language", "English").strip()
        self.icon = app.get("icon", "").strip()
        self.version = self.parse_version(app.get("version", "1.0.0.0"))

        for key, limit in LIMITS.items():
            if len(getattr(self, key)) > limit:
                fail("%s is longer than the %d characters its field holds"
                     % (key, limit))

        known, latest = api_versions()
        self.sdk = int(build.get("sdk", str(latest)).strip())

        if self.sdk not in known:
            fail("sdk = %d, and this SDK provides %s" % (
                self.sdk, ", ".join(str(n) for n in known)))

        self.sources = build.get("sources", "").split()

        if not self.sources:
            fail("%s: [build] lists no sources" % where)

        self.imports = build.get("imports", "user").split()

        for name in self.imports:
            if name not in LIBRARIES:
                fail("imports: %s is not one of %s" % (name,
                                                       " ".join(LIBRARIES)))

        self.defines = build.get("defines", "").split()
        self.includes = [os.path.join(self.root, d)
                         for d in build.get("include", "").split()]

        self.resources = []
        self.sounds = []

        if self.ini.has_section("resources"):
            for name, value in self.ini["resources"].items():
                parts = [p.strip() for p in value.split(",")]
                options = parts[1:]

                for option in options:
                    if option not in ("no-compress", "no-flip"):
                        fail("[resources] %s: %s is not no-compress or "
                             "no-flip" % (name, option))

                self.resources.append((name, os.path.join(self.root, parts[0]),
                                       options))

        if self.ini.has_section("sounds"):
            for name, value in self.ini["sounds"].items():
                self.sounds.append((name, os.path.join(self.root,
                                                       value.strip())))

        self.packaged = self.ini.has_section("package")

    @staticmethod
    def parse_version(text):
        parts = text.strip().split(".")

        if not 1 <= len(parts) <= 4 or not all(p.isdigit() for p in parts):
            fail("version %r is one to four numbers with dots between" % text)

        numbers = [int(p) for p in parts] + [0] * (4 - len(parts))

        if any(n > 65535 for n in numbers):
            fail("version %r: each number fits in sixteen bits" % text)

        return tuple(numbers)


def c_string(text):
    """A C string literal for text that came out of app.ini."""
    escaped = text.replace("\\", "\\\\").replace('"', '\\"')
    return '"%s"' % escaped


def write_boltapp(project, where):
    """boltapp.h, which modver.h includes for an application's own numbers and
    names. Written only when it would change, so an unchanged project does not
    look newer than what was compiled from it."""
    lines = [
        "#pragma once",
        "/*",
        " boltapp.h",
        " Written by bxbuild.py from %s. Edit that, not this."
        % os.path.basename(project.ini_path),
        " */",
        "",
        "#define BOLTAPP_NAME             %s" % c_string(project.name),
        "#define BOLTAPP_MODULE           %s" % c_string(project.module),
        "#define BOLTAPP_FILENAME         %s" % c_string(project.filename),
        "#define BOLTAPP_DESCRIPTION      %s" % c_string(project.description),
        "#define BOLTAPP_PRODUCT          %s" % c_string(project.product),
        "#define BOLTAPP_COPYRIGHT        %s" % c_string(project.copyright),
        "#define BOLTAPP_LANGUAGE         %s" % c_string(project.language),
        "#define BOLTAPP_ICON             %s" % c_string(project.icon),
        "",
        "#define BOLTAPP_VERSION_MAJOR    %d" % project.version[0],
        "#define BOLTAPP_VERSION_MINOR    %d" % project.version[1],
        "#define BOLTAPP_VERSION_BUILD    %d" % project.version[2],
        "#define BOLTAPP_VERSION_REVISION %d" % project.version[3],
        "#define BOLTAPP_VERSION_STRING   \"%d.%d.%d.%d\"" % project.version,
        "",
    ]

    text = "\n".join(lines)
    path = os.path.join(where, "boltapp.h")

    if os.path.exists(path) and open(path, encoding="utf-8").read() == text:
        return path

    with open(path, "w", encoding="utf-8", newline="\n") as out:
        out.write(text)

    return path


# -- Compiling and linking ------------------------------------------------------

def compile_flags(project, release, obj):
    flags = list(cflags.CL_LEADING)

    if release:
        flags += ["/O2"] + cflags.RELEASE_COMPILE
    else:
        flags += ["/Od"] + cflags.DEBUG_INFO

    defines = ["_USERMODE_APP", "BOLTSDK_APIV=%d" % project.sdk,
               "NDEBUG" if release else "_DEBUG"] + cflags.COMMON_DEFINES
    defines += project.defines

    for define in defines:
        flags += ["/D", define]

    flags += cflags.FREESTANDING + cflags.NO_BUILTIN + cflags.QUIET
    flags += cflags.CL_TRAILING
    flags += ["--target=" + toolchain.TARGET]

    # boltapp.h first, then the project's own folders, then the SDK.
    for where in [obj] + project.includes + [INCLUDE]:
        flags.append("-I" + where)

    return flags


def compile_one(tools, flags, source, obj_dir, root):
    stem = os.path.splitext(os.path.relpath(source, root))[0]
    stem = stem.replace("..", "up")
    obj = os.path.join(obj_dir, stem + ".obj")

    os.makedirs(os.path.dirname(obj), exist_ok=True)

    # The -- is for macOS, where every path starts /Users and clang-cl would
    # otherwise take the source for its /U option.
    command = [tools["clang-cl"]] + flags + ["/Fo" + obj, "--",
                                             source.replace(os.sep, "/")]
    result = subprocess.run(command, capture_output=True, text=True)
    return obj, result


def link(tools, project, release, objects, out, obj):
    target = os.path.join(out, project.filename)

    flags = ["/OUT:" + target] + cflags.LINK_COMMON
    flags += [os.path.join(LIB, name + ".lib") for name in project.imports]
    flags += ["/DEBUG", "/PDB:" + os.path.join(out, project.module + ".pdb")]
    flags += ["/SUBSYSTEM:WINDOWS"]
    flags += ["/MAP:" + os.path.join(out, project.module + ".map")]

    # An application exports its entry points, so the linker would write an
    # import library for it; that goes with the objects, since nothing links
    # against an application.
    flags += ["/IMPLIB:" + os.path.join(obj, project.module + ".lib")]

    if release:
        flags += cflags.LINK_RELEASE

    result = subprocess.run([tools["lld-link"]] + flags + objects,
                            capture_output=True, text=True)

    if result.returncode:
        print((result.stdout + result.stderr).strip()[:4000])
        fail("%s: link failed" % project.filename)

    return target


# -- What the file says ---------------------------------------------------------

def section(path, name):
    """A section's bytes, found through the section table by name, the way
    UserRes_Version and UserRes_SdkVersion find them. None when it is absent."""
    data = open(path, "rb").read()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    count = struct.unpack_from("<H", data, pe + 6)[0]
    optional = struct.unpack_from("<H", data, pe + 20)[0]

    for i in range(count):
        at = pe + 24 + optional + 40 * i

        if data[at:at + 8].rstrip(b"\0") != name:
            continue

        virtual, _, raw, where = struct.unpack_from("<IIII", data, at + 8)
        size = min(virtual, raw) if virtual else raw
        return data[where:where + size]

    return None


def check(project, target):
    """Refuse a file that does not say what app.ini says it is."""
    manifest = section(target, VERSION_SECTION)

    if manifest is None:
        fail("%s carries no version manifest. Declare one with "
             "MODVERSION_PLACE and return it from __modres; see modver.h"
             % project.filename)

    magic, size = struct.unpack_from("<II", manifest, 0)

    if magic != VERSION_MAGIC or size != VERSION_SIZE or len(manifest) < size:
        fail("%s: the version manifest is not the shape modres.h describes"
             % project.filename)

    numbers = struct.unpack_from("<4H", manifest, 8)

    if numbers != project.version:
        fail("%s says version %s, and app.ini says %s. Use MODVERSION_HEADER, "
             "which takes the numbers from boltapp.h" % (
                 project.filename, ".".join(map(str, numbers)),
                 ".".join(map(str, project.version))))

    offset, width = VERSION_NAME
    said = manifest[offset:offset + width].split(b"\0")[0].decode("ascii",
                                                                  "replace")

    if said != project.filename:
        fail("%s calls itself %s. Use BOLTAPP_FILENAME for the original "
             "filename" % (project.filename, said))

    record = section(target, SDK_SECTION)

    if record is None or len(record) < 12:
        fail("%s carries no SDK record; was it built with the SDK's headers?"
             % project.filename)

    magic, _, version = struct.unpack_from("<III", record, 0)

    if magic != SDK_MAGIC or version != project.sdk:
        fail("%s records SDK %d, and app.ini says %d" % (
            project.filename, version, project.sdk))


def attach(project, target):
    """The pictures and sounds the application carries, written past the end
    of the file. After the link and the check, since a link writes the file
    afresh. See modres.h for why the loader does not mind."""
    items = []

    for name, source, options in project.resources:
        if not os.path.isfile(source):
            fail("[resources] %s: no file at %s" % (name, source))

        items.append(multibmpres.bitmap_item(source, name,
                                             "no-compress" not in options,
                                             "no-flip" not in options))

    for name, source in project.sounds:
        if not os.path.isfile(source):
            fail("[sounds] %s: no file at %s" % (name, source))

        items.append(multibmpres.sound_item(source, name))

    if not items:
        return 0, 0

    return len(items), multibmpres.attach(target, items)


def package(project, target, out):
    path = os.path.join(out, project.module + ".bxi")
    contents = mkbxi.package_from_ini(project.ini_path, target)
    size = mkbxi.write(contents, path, out)

    # Read straight back, the whole package walked and every member inflated
    # and checked, which is what says it is the shape bxi.h describes.
    with contextlib.redirect_stdout(io.StringIO()):
        mkbxi.check(path)

    return path, size


# -- Going ----------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("project", help="the project folder, or its app.ini")
    parser.add_argument("--release", action="store_true",
                        help="optimise, and leave out the DWARF")
    parser.add_argument("--clean", action="store_true",
                        help="remove the output folder first")
    args = parser.parse_args(argv)

    project = Project(args.project)
    configuration = "release" if args.release else "debug"
    out = os.path.join(project.root, "out", configuration)
    obj = os.path.join(out, "obj")

    if args.clean and os.path.isdir(out):
        shutil.rmtree(out)

    os.makedirs(obj, exist_ok=True)

    for name in project.imports:
        if not os.path.isfile(os.path.join(LIB, name + ".lib")):
            fail("lib/%s.lib is missing from the SDK" % name)

    tools = toolchain.require("clang-cl", "lld-link")

    print("%s %s, %s, Bolt SDK %d" % (project.name,
                                      ".".join(map(str, project.version)),
                                      configuration, project.sdk))
    print("  toolchain    %s" % tools["clang-cl"])

    write_boltapp(project, obj)
    flags = compile_flags(project, args.release, obj)
    sources = [os.path.join(project.root, s) for s in project.sources]

    for source in sources:
        if not os.path.isfile(source):
            fail("no source at %s" % source)

    # What an editor needs to see the defines and the include paths the
    # compile actually used.
    with open(os.path.join(out, "compile_commands.json"), "w",
              encoding="utf-8", newline="\n") as commands:
        json.dump([{"directory": project.root, "file": s,
                    "arguments": [tools["clang-cl"]] + flags + ["--", s]}
                   for s in sources], commands, indent=1)

    objects = []
    failed = False

    with concurrent.futures.ThreadPoolExecutor(
            max_workers=os.cpu_count()) as pool:
        futures = [pool.submit(compile_one, tools, flags, s, obj, project.root)
                   for s in sources]

        for source, future in zip(sources, futures):
            result_obj, result = future.result()
            objects.append(result_obj)
            print("  %s" % os.path.relpath(source, project.root))

            for line in (result.stdout + result.stderr).splitlines():
                if line.strip():
                    print("    " + line)

            if result.returncode:
                failed = True

    if failed:
        fail("compile failed")

    target = link(tools, project, args.release, objects, out, obj)
    check(project, target)
    count, size = attach(project, target)

    print("  %-12s %d bytes" % (project.filename, os.path.getsize(target)))

    if count:
        print("  %-12s %d resources, %d bytes" % ("carries", count, size))

    if project.packaged:
        path, size = package(project, target, out)
        print("  %-12s %d bytes" % (os.path.basename(path), size))

    print()
    print("built %s" % os.path.relpath(target, os.getcwd()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
