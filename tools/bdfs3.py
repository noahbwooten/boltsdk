#!/usr/bin/env python
"""
bdfs3.py
(c)Noah Wooten 2023 - 2026, All Rights Reserved

The BDFS3 image tool. Replaces bdfs2.exe, whose source was lost, and matches
the layout in kernel/kernel/include/bdfs3.h. Anything changed there has to be
changed here in the same pass.

  bdfs3.py <image> <hostfile> <fspath>   add a file, making the image if absent
  bdfs3.py <image> --list                what the image holds
  bdfs3.py <image> --format [megabytes]  an empty volume, nothing else
  bdfs3.py <image> --attrib <rhs> <path> [path...]   set attributes
  bdfs3.py <image> --remove <path> [path...]        take a file out

Attributes are the letters r, h and s, or a dash for none. They are replaced
whole rather than added to, so what is written is what was asked for.

Every file and folder the tool makes is stamped as it is made: created and
accessed then, and modified when the host file was last written, so a module
says when it was built. The times are UTC, which is what the kernel stamps
(KmCore->KernelInfo.EpochTime); readers localize for display.
"""

import os
import struct
import sys
import time

BLOCKSIZE = 4096
MAGIC = 0x33464442
VERSION = 1
ROOTINODE = 1

TYPE_FREE, TYPE_FILE, TYPE_DIR = 0, 1, 2

# The inode's Flags, as bdfs3.h names them.
ATTR = {"r": 1, "h": 2, "s": 4}

DIRECT = 9
INODESIZE = 128
INODESPERBLOCK = BLOCKSIZE // INODESIZE

DEFAULT_MB = 64

SUPER = "<12I Q 18I"
INODE = "<II QQQ IIII" + "II" * DIRECT + "II"

# Where the three times sit among an inode's fields as INODE unpacks them.
# Accessed is the last word, which bdfs3.h used to call Reserved.
CREATED, MODIFIED, ACCESSED = 3, 4, 9 + DIRECT * 2 + 1


def machine_seconds(when=None):
    """Seconds since 1970, UTC. The kernel stamps KmCore->KernelInfo.EpochTime,
    which counts UTC, and every reader localizes for display, so a local time
    written here would read hours out on the machine. Kept to thirty-two bits,
    which is what Accessed has room for."""
    return int(when if when is not None else time.time()) & 0xFFFFFFFF


class Volume(object):
    """An image held in memory and written back whole. The images this builds
    are tens of megabytes, so holding one costs less than seeking around it."""

    def __init__(self, path):
        self.path = path

        # One time for everything this opening of the image makes, so files
        # added together say they were made together.
        self.stamp = machine_seconds()
        with open(path, "rb") as f:
            self.data = bytearray(f.read())
        self.read_super()

    # -- the superblock --

    def read_super(self):
        v = struct.unpack_from(SUPER, self.data, 0)
        (self.magic, self.version, self.blocksize, self.total,
         self.bitmap_block, self.bitmap_blocks, self.inode_block,
         self.inode_count, self.root, self.free, self.mirror,
         self.clean) = v[:12]
        self.generation = v[12]

        if self.magic != MAGIC:
            raise SystemExit("%s is not a BDFS3 volume" % self.path)

    def write_super(self):
        self.generation += 1
        packed = struct.pack(SUPER, MAGIC, VERSION, BLOCKSIZE, self.total,
                             self.bitmap_block, self.bitmap_blocks,
                             self.inode_block, self.inode_count, ROOTINODE,
                             self.free, self.mirror, 1, self.generation,
                             *([0] * 18))
        # The primary first and the mirror after, the same order the kernel
        # uses, so an image cut short here looks the way a machine cut short
        # there does.
        self.data[0:len(packed)] = packed
        off = self.mirror * BLOCKSIZE
        self.data[off:off + len(packed)] = packed

    def save(self):
        self.write_super()
        with open(self.path, "wb") as f:
            f.write(self.data)

    # -- blocks --

    def block(self, n):
        return n * BLOCKSIZE

    def bit_get(self, n):
        off = self.bitmap_block * BLOCKSIZE + (n >> 3)
        return (self.data[off] >> (n & 7)) & 1

    def bit_set(self, n, value):
        off = self.bitmap_block * BLOCKSIZE + (n >> 3)
        if value:
            self.data[off] |= (1 << (n & 7))
        else:
            self.data[off] &= ~(1 << (n & 7)) & 0xFF

    def alloc_run(self, count):
        """A run of consecutive free blocks. First fit, which on an image built
        once from empty is also best fit."""
        run = 0
        start = 0

        for b in range(self.total):
            if self.bit_get(b):
                run = 0
                continue

            if not run:
                start = b
            run += 1

            if run == count:
                for i in range(start, start + count):
                    self.bit_set(i, 1)
                self.free -= count
                return start

        raise SystemExit("bdfs3: the volume is full")

    # -- inodes --

    def inode_off(self, n):
        return self.inode_block * BLOCKSIZE + n * INODESIZE

    def read_inode(self, n):
        return list(struct.unpack_from(INODE, self.data, self.inode_off(n)))

    def write_inode(self, n, fields):
        struct.pack_into(INODE, self.data, self.inode_off(n), *fields)

    def new_inode(self, kind):
        for n in range(2, self.inode_count):
            if self.read_inode(n)[0] == TYPE_FREE:
                fields = ([kind, 1, 0, 0, 0, 0, 0, 0, 0]
                          + [0] * (DIRECT * 2) + [0, 0])
                fields[CREATED] = fields[MODIFIED] = self.stamp
                fields[ACCESSED] = self.stamp
                self.write_inode(n, fields)
                return n

        raise SystemExit("bdfs3: no free inodes")

    # -- file data --

    def inode_extents(self, fields):
        out = []
        for i in range(DIRECT):
            start, count = fields[9 + i * 2], fields[10 + i * 2]
            if count:
                out.append((start, count))
        return out

    def set_data(self, n, payload):
        """One run holding the whole file. The imager never rewrites a file, so
        it never needs the second extent or the indirect block."""
        fields = self.read_inode(n)
        blocks = (len(payload) + BLOCKSIZE - 1) // BLOCKSIZE

        if blocks:
            start = self.alloc_run(blocks)
            off = self.block(start)
            self.data[off:off + len(payload)] = payload
            fields[9], fields[10] = start, blocks

        fields[2] = len(payload)

        # Written now, which is what the kernel says of anything it writes. A
        # file added from the host has its own modified time put back after.
        fields[MODIFIED] = fields[ACCESSED] = self.stamp
        self.write_inode(n, fields)

    def read_data(self, n):
        fields = self.read_inode(n)
        size = fields[2]
        out = bytearray()

        for start, count in self.inode_extents(fields):
            off = self.block(start)
            out += self.data[off:off + count * BLOCKSIZE]

        return bytes(out[:size])

    # -- directories --

    def dir_entries(self, n):
        """Every live entry of a directory, as (inode, type, name)."""
        raw = self.read_data(n)
        out = []
        pos = 0

        while pos + 8 <= len(raw):
            ino, length, namelen, kind = struct.unpack_from("<IHBB", raw, pos)
            if not length:
                # The zero padding to the end of a block, not the end of the
                # directory. No record straddles a block, so the next one
                # starts at the next boundary. Stopping here instead read only
                # the first block, and dir_write then laid the directory out
                # again from what had been read: adding a file to a directory
                # the machine had grown past one block deleted everything in
                # the blocks after it.
                pos = ((pos // BLOCKSIZE) + 1) * BLOCKSIZE
                continue
            if ino:
                name = raw[pos + 8:pos + 8 + namelen].decode("utf-8", "replace")
                out.append((ino, kind, name))
            pos += length

        return out

    def dir_write(self, n, entries):
        """A directory laid out again from its entries. No record straddles a
        block, so a reader takes one block at a time and never joins two."""
        blocks = []
        cur = bytearray()

        for ino, kind, name in entries:
            raw = name.encode("utf-8")
            length = (8 + len(raw) + 3) & ~3

            if len(cur) + length > BLOCKSIZE:
                cur += b"\0" * (BLOCKSIZE - len(cur))
                blocks.append(bytes(cur))
                cur = bytearray()

            rec = struct.pack("<IHBB", ino, length, len(raw), kind) + raw
            cur += rec + b"\0" * (length - len(rec))

        if cur or not blocks:
            cur += b"\0" * (BLOCKSIZE - len(cur))
            blocks.append(bytes(cur))

        # Freed before the new run is taken, so a directory that grew by one
        # entry reuses the blocks it just gave up.
        fields = self.read_inode(n)
        for start, count in self.inode_extents(fields):
            for b in range(start, start + count):
                self.bit_set(b, 0)
                self.free += 1

        for i in range(DIRECT * 2):
            fields[9 + i] = 0
        self.write_inode(n, fields)

        self.set_data(n, b"".join(blocks))

    def dir_find(self, n, name):
        for ino, kind, entry in self.dir_entries(n):
            if entry.lower() == name.lower():
                return ino, kind
        return 0, 0

    def dir_add(self, n, name, ino, kind):
        entries = self.dir_entries(n)
        entries.append((ino, kind, name))
        self.dir_write(n, entries)

    # -- paths --

    def resolve(self, path, create_dirs=False):
        """The inode a path names, making every missing component as a
        directory when asked. Matching ignores case, the way the registry
        does."""
        parts = [p for p in path.split("/") if p]
        node = ROOTINODE

        for part in parts:
            found, kind = self.dir_find(node, part)

            if found:
                node = found
                continue

            if not create_dirs:
                return 0

            made = self.new_inode(TYPE_DIR)
            self.dir_write(made, [])
            self.dir_add(node, part, made, TYPE_DIR)
            node = made

        return node

    def add_file(self, hostpath, fspath):
        with open(hostpath, "rb") as f:
            payload = f.read()

        parts = [p for p in fspath.split("/") if p]
        if not parts:
            raise SystemExit("bdfs3: '%s' names no file" % fspath)

        parent = self.resolve("/".join(parts[:-1]), create_dirs=True) \
            if len(parts) > 1 else ROOTINODE

        if not parent:
            raise SystemExit("bdfs3: could not make the path to '%s'" % fspath)

        existing, _ = self.dir_find(parent, parts[-1])
        if existing:
            raise SystemExit("bdfs3: '%s' is already there" % fspath)

        ino = self.new_inode(TYPE_FILE)
        self.set_data(ino, payload)

        # When the host file was last written, which for a module is when it
        # was linked. Created stays the moment it went into the image.
        fields = self.read_inode(ino)
        fields[MODIFIED] = machine_seconds(os.path.getmtime(hostpath))
        self.write_inode(ino, fields)

        self.dir_add(parent, parts[-1], ino, TYPE_FILE)

        return len(payload)

    def remove_file(self, fspath):
        """Take a file out of its directory and free what it held. The blocks
        go back to the bitmap and the inode to the free list, so the image is
        the same size it would have been had the file never been added."""
        parts = [p for p in fspath.split("/") if p]
        if not parts:
            raise SystemExit("bdfs3: '%s' names no file" % fspath)

        parent = self.resolve("/".join(parts[:-1])) if len(parts) > 1             else ROOTINODE

        if not parent:
            raise SystemExit("bdfs3: there is no path to '%s'" % fspath)

        ino, kind = self.dir_find(parent, parts[-1])
        if not ino:
            raise SystemExit("bdfs3: there is no '%s' to remove" % fspath)
        if kind == TYPE_DIR:
            raise SystemExit("bdfs3: '%s' is a directory" % fspath)

        fields = self.read_inode(ino)

        # Counted back into the superblock as dir_write does, or a volume that
        # has a file replaced on it again and again -- which is what the SDK's
        # slipstreaming does -- reports less free space every time.
        for start, count in self.inode_extents(fields):
            for b in range(count):
                self.bit_set(start + b, 0)
                self.free += 1

        self.write_inode(ino, [0] * len(fields))

        entries = [e for e in self.dir_entries(parent) if e[0] != ino]
        self.dir_write(parent, entries)

    # -- attributes --

    def set_attributes(self, fspath, flags):
        """Flags 8 of the inode, replaced whole. The imager sets them after
        the file is written, so nothing has to know them while it is."""
        ino = self.resolve(fspath)

        if not ino:
            raise SystemExit("bdfs3: there is no '%s' to mark" % fspath)

        fields = self.read_inode(ino)
        fields[8] = flags
        self.write_inode(ino, fields)

    def attributes(self, ino):
        return self.read_inode(ino)[8]

    def walk(self, node, prefix, out):
        for ino, kind, name in sorted(self.dir_entries(node), key=lambda e: e[2]):
            path = prefix + "/" + name
            attr = self.attributes(ino)
            if kind == TYPE_DIR:
                out.append((path + "/", 0, attr))
                self.walk(ino, path, out)
            else:
                out.append((path, self.read_inode(ino)[2], attr))


def attribute_flags(letters):
    """The letters as the inode holds them. A dash is none of them, which is
    how an attribute is taken off again."""
    if letters == "-":
        return 0

    flags = 0
    for c in letters.lower():
        if c not in ATTR:
            raise SystemExit("bdfs3: '%s' is not an attribute letter" % c)
        flags |= ATTR[c]

    return flags


def attribute_text(flags):
    out = "".join(c.upper() for c in "rhs" if flags & ATTR[c])
    return out if out else "-"


def format_volume(path, megabytes):
    total = (megabytes * 1024 * 1024) // BLOCKSIZE

    # One inode per eight blocks, which is a file every thirty two kilobytes.
    inode_count = max(256, total // 8)
    inode_blocks = (inode_count + INODESPERBLOCK - 1) // INODESPERBLOCK
    inode_count = inode_blocks * INODESPERBLOCK

    bitmap_blocks = (((total + 7) // 8) + BLOCKSIZE - 1) // BLOCKSIZE

    bitmap_block = 1
    inode_block = bitmap_block + bitmap_blocks
    first_free = inode_block + inode_blocks
    mirror = total - 1

    data = bytearray(total * BLOCKSIZE)

    packed = struct.pack(SUPER, MAGIC, VERSION, BLOCKSIZE, total,
                         bitmap_block, bitmap_blocks, inode_block, inode_count,
                         ROOTINODE, total - first_free - 1, mirror, 1, 1,
                         *([0] * 18))
    data[0:len(packed)] = packed
    data[mirror * BLOCKSIZE:mirror * BLOCKSIZE + len(packed)] = packed

    with open(path, "wb") as f:
        f.write(data)

    v = Volume(path)

    # The metadata blocks and the mirror, marked used before anything is added.
    for b in list(range(first_free)) + [mirror]:
        v.bit_set(b, 1)
    v.free = total - first_free - 1

    root = [TYPE_DIR, 1, 0, 0, 0, 0, 0, 0, 0] + [0] * (DIRECT * 2) + [0, 0]
    root[CREATED] = root[MODIFIED] = root[ACCESSED] = v.stamp
    v.write_inode(ROOTINODE, root)
    v.dir_write(ROOTINODE, [])
    v.save()

    return v


def main(argv):
    if len(argv) < 3:
        print(__doc__.strip())
        return 1

    image = argv[1]

    if argv[2] == "--format":
        mb = int(argv[3]) if len(argv) > 3 else DEFAULT_MB
        v = format_volume(image, mb)
        print("bdfs3: %s formatted, %d MB, %d inodes, %d free blocks"
              % (image, mb, v.inode_count, v.free))
        return 0

    if argv[2] == "--list":
        v = Volume(image)
        out = []
        v.walk(ROOTINODE, "", out)
        for path, size, attr in out:
            print("  %-44s %-5s %s"
                  % (path, attribute_text(attr),
                     "" if path.endswith("/") else size))
        used = v.total - v.free
        print("  %d files, %d of %d blocks used, generation %d"
              % (sum(1 for p, _, _ in out if not p.endswith("/")),
                 used, v.total, v.generation))
        return 0

    if argv[2] == "--remove":
        if len(argv) < 4:
            print("bdfs3: --remove wants at least one path")
            return 2

        v = Volume(image)

        for path in argv[3:]:
            v.remove_file(path)
            print("removed %s" % path)

        v.save()
        return 0

    if argv[2] == "--attrib":
        if len(argv) < 5:
            print(__doc__.strip())
            return 1

        flags = attribute_flags(argv[3])
        v = Volume(image)

        for path in argv[4:]:
            v.set_attributes(path, flags)
            print("  %-44s %s" % (path, attribute_text(flags)))

        v.save()
        return 0

    if len(argv) < 4:
        print(__doc__.strip())
        return 1

    if not os.path.exists(image):
        format_volume(image, DEFAULT_MB)

    v = Volume(image)
    size = v.add_file(argv[2], argv[3])
    v.save()
    print("  %-44s %d bytes" % (argv[3], size))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
