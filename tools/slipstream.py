"""Put an application onto the SDK's disk.

    python tools/slipstream.py --disk run/bolt-disk.img --image image/fs.bin
                               [--clean] [what ...]

What can be given, any number of them, in any order:

    examples/calc          a project folder: its newest build, named and drawn
                           the way its app.ini says
    out/debug/calc.bxf     an application, by itself
    out/debug/calc.bxi     a package, which goes on the desktop to be installed
    notes.txt=/users/default/desktop/notes.txt
                           any file, put exactly where it is told

An application goes into /programs/<module>/<module>.bxf, which is where the
installer would put it, with a link at the top of the Actions menu. A package
goes on the desktop, and installing it is double-clicking it, which runs the
installer the way it runs for anybody. Anything already at a path is replaced.

The disk is made from the image the first time, and again with --clean, which
throws away everything the machine wrote to it. run.cmd and run.sh call this
before they start the machine; there is rarely a reason to call it yourself.
"""

import argparse
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SDK = os.path.dirname(HERE)

sys.path.insert(0, HERE)

import bdfs3
import mkbxi
import mklink
import multibmpres

from bxbuild import section, VERSION_SECTION

# Where things go, as folders.h has them.
PROGRAMS = "/programs"
ACTIONS = "/users/default/actions"
DESKTOP = "/users/default/desktop"

# How large a disk is made, whatever the image is. The volume inside is the
# image's size; the rest is room the machine writes past, the same size
# run_qemu.py makes its disks.
DISK_BYTES = 64 * 1024 * 1024


def fail(message):
    raise SystemExit("slipstream: " + message)


def make_disk(disk, image):
    """A fresh disk: the image, then zeros to DISK_BYTES."""
    if not os.path.isfile(image):
        fail("no image at %s" % image)

    payload = open(image, "rb").read()
    size = max(DISK_BYTES, (len(payload) + 511) // 512 * 512)

    os.makedirs(os.path.dirname(os.path.abspath(disk)), exist_ok=True)

    with open(disk, "wb") as out:
        out.write(payload)
        out.write(b"\0" * (size - len(payload)))

    print("  a fresh disk from %s" % os.path.relpath(image))


def in_use(disk):
    """Whether a running machine has the disk open. Windows will not rename a
    file QEMU holds, and writing under a running machine is how a volume gets
    corrupted, so a rename that fails is the answer. Elsewhere nothing holds a
    file like that, and the answer is always no."""
    if not os.path.exists(disk):
        return False

    moved = disk + ".check"

    try:
        os.rename(disk, moved)
    except OSError:
        return True

    os.rename(moved, disk)
    return False


def replace(volume, host, path):
    """One file onto the volume, taking whatever was at its path first."""
    parent = "/".join(path.split("/")[:-1])
    node = volume.resolve(parent) if parent else bdfs3.ROOTINODE

    if node:
        found, kind = volume.dir_find(node, path.split("/")[-1])

        if found and kind == bdfs3.TYPE_DIR:
            fail("%s is a folder on the disk" % path)

        if found:
            volume.remove_file(path)

    return volume.add_file(host, path)


def describe(module):
    """A module's name and icon, from the file alone: the description in its
    version manifest, and the first AppIcon_ picture it carries."""
    name = os.path.splitext(os.path.basename(module))[0]
    icon = ""

    manifest = section(module, VERSION_SECTION)

    if manifest and len(manifest) >= 88:
        said = manifest[24:88].split(b"\0")[0].decode("ascii", "replace")
        name = said or name

    for item in multibmpres.carried(module) or []:
        if item[0].startswith("AppIcon_"):
            icon = item[0]
            break

    return name, icon


def newest_build(project):
    """A project's .bxf, from whichever of its two builds was made last."""
    ini = mkbxi.read_ini(os.path.join(project, "app.ini"))

    if not ini.has_section("application"):
        fail("%s: app.ini has no [application]" % project)

    app = ini["application"]
    module = app.get("module", "").strip()

    builds = [os.path.join(project, "out", c, module + ".bxf")
              for c in ("debug", "release")]
    builds = [b for b in builds if os.path.isfile(b)]

    if not builds:
        fail("%s has not been built; build.cmd %s" % (project, project))

    builds.sort(key=os.path.getmtime)
    return builds[-1], app.get("name", module).strip(), app.get("icon", "").strip()


def put_application(volume, module, name, icon):
    stem = os.path.splitext(os.path.basename(module))[0]
    target = "%s/%s/%s.bxf" % (PROGRAMS, stem, stem)

    size = replace(volume, module, target)
    print("  %-40s %d bytes" % (target, size))

    # The link, written beside the disk rather than into a temporary folder,
    # so a failure leaves it where it can be looked at.
    link = os.path.join(os.path.dirname(os.path.abspath(volume.path)),
                        stem + mklink.SUFFIX)
    mklink.write_link(link, name, target, icon)

    where = "%s/%s%s" % (ACTIONS, stem, mklink.SUFFIX)
    replace(volume, link, where)
    os.remove(link)

    print("  %-40s \"%s\"%s" % (where, name, ", " + icon if icon else ""))


def put_package(volume, package):
    target = "%s/%s" % (DESKTOP, os.path.basename(package))
    size = replace(volume, package, target)
    print("  %-40s %d bytes" % (target, size))


def put_file(volume, host, target):
    if not target.startswith("/"):
        fail("%s: where it goes on the disk starts with /" % target)

    size = replace(volume, host, target)
    print("  %-40s %d bytes" % (target, size))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--disk", required=True,
                        help="the disk the machine boots, made if absent")
    parser.add_argument("--image", required=True,
                        help="the volume a fresh disk is made from")
    parser.add_argument("--clean", action="store_true",
                        help="make the disk again from the image first")
    parser.add_argument("what", nargs="*",
                        help="projects, .bxf, .bxi, or host=/path pairs")
    args = parser.parse_args(argv)

    if in_use(args.disk):
        fail("%s is in use; close the machine that has it open"
             % os.path.basename(args.disk))

    if args.clean or not os.path.exists(args.disk):
        make_disk(args.disk, args.image)

    if not args.what:
        return 0

    volume = bdfs3.Volume(args.disk)

    for what in args.what:
        host, equals, target = what.partition("=")

        if equals:
            if not os.path.isfile(host):
                fail("no file at %s" % host)
            put_file(volume, host, target)
        elif os.path.isdir(what):
            if not os.path.isfile(os.path.join(what, "app.ini")):
                fail("%s is a folder with no app.ini" % what)
            module, name, icon = newest_build(what)
            put_application(volume, module, name, icon)
        elif what.lower().endswith(".bxf"):
            if not os.path.isfile(what):
                fail("no file at %s" % what)
            name, icon = describe(what)
            put_application(volume, what, name, icon)
        elif what.lower().endswith(".bxi"):
            if not os.path.isfile(what):
                fail("no file at %s" % what)
            put_package(volume, what)
        else:
            fail("%s is not a project, a .bxf, a .bxi or a host=/path pair"
                 % what)

    volume.save()
    return 0


if __name__ == "__main__":
    sys.exit(main())
