"""BoltOS resource bundles.

    python scripts/multibmpres.py <bundle> --add [options] <bmp> <name>
        options: --no-compress, --no-flip
    python scripts/multibmpres.py <bundle> --add-sound <wav> <name>
    python scripts/multibmpres.py <bundle> --list

Packs 24-bit bitmaps and PCM sounds into the .res bundle the shell and the
kernel read. This replaces multibmpres.exe, a Win32 binary built from a
separate repository, for the same reason bdfs3.py replaced bdfs2.exe: a build
that cannot run without a prebuilt tool cannot run on a machine that tool was
not built for.

The format is mirrored from kernel/kernel/include/res.h and the reader in
kernel/func/gui/resources.c. A field added there has to be added below.

The layout is a header holding the offset of the first record, then records and
their payloads alternating, then one zeroed record to end the walk. A record
names the data that follows it and where the next record begins, so the bundle
is a linked list laid out in file order.
"""

import os
import struct
import sys

HEADER = 4     # RESOURCE_LIST: the offset of the first record
RECORD = 280   # RESOURCE_ITEM, padded to 4 as the compiler lays it out

NAME = 256     # ResName, NUL terminated

# What the payload is. Kind occupies a byte the record was already padding
# with, so the record size is unchanged and a bundle written before it existed
# reads back as bitmaps, which is what it held.
RESKIND_BITMAP = 0
RESKIND_SOUND = 1

assert struct.calcsize("<IIIIBB2xI") + NAME == RECORD

# What the card is set to in hal/uefi/audio.c. Sounds are converted to it here
# rather than in the kernel: the conversion happens once, on a machine with a
# Python interpreter, instead of on every boot in code with no floating point.
SOUND_RATE = 48000


def compress(raw):
    """Byte-pair run length, matching MultiBmpClient_Decompress.

    Every pair is a count and a value, so a run cannot exceed 255 and there is
    no escape for literal bytes. That makes the worst case twice the input,
    which is why the caller is allowed to say --no-compress."""
    out = bytearray()
    i = 0

    while i < len(raw):
        value = raw[i]
        run = 1

        while i + run < len(raw) and raw[i + run] == value and run < 255:
            run += 1

        out.append(run)
        out.append(value)
        i += run

    return bytes(out)


def decompress(data):
    """The inverse, used only by --list to report a resource's real size."""
    out = bytearray()

    for i in range(0, len(data) - 1, 2):
        out += bytes([data[i + 1]]) * data[i]

    return bytes(out)


def read_bmp(path, flip):
    """One bitmap, as the packed rows the shell blits.

    A BMP stores its rows bottom up and pads each to four bytes. Both are
    undone here, so what reaches the bundle is width * height * 3 with no
    stride, in the file's own byte order. --no-flip leaves the rows as they
    are, which is what aboutico.bmp wants."""
    data = open(path, "rb").read()

    if data[:2] != b"BM":
        raise ValueError("%s: not a bitmap" % path)

    start, = struct.unpack_from("<I", data, 10)
    width, height = struct.unpack_from("<ii", data, 18)
    bpp, = struct.unpack_from("<H", data, 28)
    compression, = struct.unpack_from("<I", data, 30)

    if compression != 0:
        raise ValueError("%s: compressed bitmaps are not read" % path)

    # A negative height already means top down, so the rows are in the order
    # wanted and reversing them would put them back upside down.
    top_down = height < 0
    height = abs(height)

    if bpp == 24:
        stride = (width * 3 + 3) // 4 * 4
        rows = [data[start + y * stride:start + y * stride + width * 3]
                for y in range(height)]
    elif bpp in (1, 4, 8):
        rows = paletted(path, data, width, height, bpp)
    else:
        raise ValueError("%s: %d bits per pixel, expected 24 or a palette"
                         % (path, bpp))

    for y, row in enumerate(rows):
        if len(row) < width * 3:
            raise ValueError("%s: truncated at row %d" % (path, y))

    if flip and not top_down:
        rows.reverse()

    return width, height, b"".join(rows)


def paletted(path, data, width, height, bpp):
    """A 1, 4 or 8 bit bitmap, expanded to the triples the rest of this reads.

    Editors save a small picture with few colours as a paletted bitmap without
    being asked, so refusing one means an icon has to be converted by hand
    before it can be packed. What comes back is byte for byte what the same
    picture saved at 24 bits would have been: a palette entry is stored blue,
    green, red, alpha, and the first three of those are the order a 24 bit row
    is already in."""
    start, = struct.unpack_from("<I", data, 10)
    header, = struct.unpack_from("<I", data, 14)

    if header < 40:
        raise ValueError("%s: %d byte header is too old to carry a palette"
                         % (path, header))

    colours, = struct.unpack_from("<I", data, 46)
    if not colours:
        colours = 1 << bpp

    table = 14 + header
    palette = [data[table + (i * 4):table + (i * 4) + 3] for i in range(colours)]

    if len(palette) < colours or len(palette[-1]) < 3:
        raise ValueError("%s: palette is short of %d entries" % (path, colours))

    # Rows are padded to four bytes whatever the depth, so the stride is taken
    # from the bit width rather than from the byte width.
    stride = ((width * bpp) + 31) // 32 * 4
    per = 8 // bpp
    mask = (1 << bpp) - 1

    rows = []

    for y in range(height):
        base = start + (y * stride)

        if base + stride > len(data):
            raise ValueError("%s: truncated at row %d" % (path, y))

        row = bytearray()

        for x in range(width):
            packed = data[base + (x // per)]
            index = (packed >> (8 - bpp - ((x % per) * bpp))) & mask

            if index >= colours:
                raise ValueError("%s: row %d names colour %d of %d"
                                 % (path, y, index, colours))

            row += palette[index]

        rows.append(bytes(row))

    return rows


def read_wav(path):
    """One sound, as the 16-bit samples the mixer adds together.

    Returns (channels, pcm). Whatever the file was, what comes back is signed
    16-bit little endian at SOUND_RATE, so nothing downstream converts anything:
    the mixer walks samples and the card is handed them. The channel count is
    kept rather than forced, since a mono sound is half the size and most of
    these are mono."""
    data = open(path, "rb").read()

    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError("%s: not a RIFF WAVE file" % path)

    fmt = None
    pcm = None
    offset = 12

    # A WAVE file is a list of chunks and the two that matter are not required
    # to be adjacent or in any order, so this walks rather than assuming.
    while offset + 8 <= len(data):
        chunk = data[offset:offset + 4]
        size, = struct.unpack_from("<I", data, offset + 4)
        body = data[offset + 8:offset + 8 + size]

        if chunk == b"fmt ":
            fmt = struct.unpack_from("<HHIIHH", body, 0)
        elif chunk == b"data":
            pcm = body

        # Chunks are padded to even lengths and the pad is not counted in size.
        offset += 8 + size + (size & 1)

    if not fmt or pcm is None:
        raise ValueError("%s: no fmt or data chunk" % path)

    encoding, channels, rate, _, _, bits = fmt

    # 0xFFFE is WAVE_FORMAT_EXTENSIBLE, which is PCM with a longer fmt chunk
    # naming a speaker layout. The samples are laid out identically.
    if encoding not in (1, 0xFFFE):
        raise ValueError("%s: encoding %d is not PCM" % (path, encoding))

    if channels not in (1, 2):
        raise ValueError("%s: %d channels, expected 1 or 2" % (path, channels))

    if bits == 8:
        # 8-bit WAV is unsigned and centred on 128, unlike every other width.
        samples = [(b - 128) * 256 for b in pcm]
    elif bits == 16:
        count = len(pcm) // 2
        samples = list(struct.unpack_from("<%dh" % count, pcm, 0))
    else:
        raise ValueError("%s: %d bits per sample, expected 8 or 16" % (path, bits))

    if rate != SOUND_RATE:
        samples = resample(samples, channels, rate, SOUND_RATE)

    return channels, struct.pack("<%dh" % len(samples), *samples)


def resample(samples, channels, source_rate, target_rate):
    """Linear interpolation between the two nearest frames.

    Good enough for short interface sounds and wrong for music, which is not
    what a resource bundle is for. Each channel is interpolated on its own, so
    a stereo image survives."""
    frames = len(samples) // channels
    out_frames = (frames * target_rate) // source_rate

    out = []

    for i in range(out_frames):
        # Where this output frame falls between two input frames, in 16.16
        # fixed point, so the whole conversion stays in integers.
        position = (i * source_rate * 65536) // target_rate
        index = position >> 16
        weight = position & 0xFFFF

        following = index + 1 if index + 1 < frames else index

        for c in range(channels):
            first = samples[index * channels + c]
            second = samples[following * channels + c]

            out.append(first + (((second - first) * weight) >> 16))

    return out


def read_bundle(path):
    """Every record in a bundle, in file order.

    Returns a list of (name, width, height, nocompress, kind, payload). The
    payload is kept exactly as stored, so a bundle read and written back is
    unchanged byte for byte."""
    if not os.path.exists(path):
        return []

    data = open(path, "rb").read()

    if len(data) < HEADER + RECORD:
        raise ValueError("%s: too small to hold a record" % path)

    items = []
    offset, = struct.unpack_from("<I", data, 0)

    while offset + RECORD <= len(data):
        name = data[offset:offset + NAME].split(b"\0")[0].decode("ascii")
        location, size, width, height, nocompress, kind, following = \
            struct.unpack_from("<IIIIBB2xI", data, offset + NAME)

        # The zeroed record the writer leaves at the end. Nothing follows it.
        if not location:
            break

        items.append((name, width, height, nocompress, kind,
                      data[location:location + size]))

        if following <= offset:
            raise ValueError("%s: record at %d does not advance" % (path, offset))

        offset = following

    return items


def bundle_bytes(items):
    """A whole bundle, laid out from scratch.

    Rewritten rather than appended to. The layout is decided entirely by the
    order and the sizes, so writing all of it produces what appending would
    have, and there is one piece of code that knows the arrangement."""
    out = bytearray(struct.pack("<I", HEADER))

    for name, width, height, nocompress, kind, payload in items:
        encoded = name.encode("ascii")

        if len(encoded) >= NAME:
            raise ValueError("%r does not fit in %d bytes" % (name, NAME))

        location = len(out) + RECORD

        out += encoded + b"\0" * (NAME - len(encoded))
        out += struct.pack("<IIIIBB2xI", location, len(payload), width, height,
                           1 if nocompress else 0, kind,
                           location + len(payload))
        out += payload

    out += b"\0" * RECORD

    return bytes(out)


def write_bundle(path, items):
    open(path, "wb").write(bundle_bytes(items))


def add(path, bmp, name, do_compress, flip):
    items = read_bundle(path)

    if any(existing == name for existing, _, _, _, _, _ in items):
        raise ValueError("%s: %s is already in the bundle" % (path, name))

    width, height, raw = read_bmp(bmp, flip)
    payload = compress(raw) if do_compress else raw

    items.append((name, width, height, not do_compress, RESKIND_BITMAP, payload))
    write_bundle(path, items)

    print("  %-34s %4dx%-4d %7d bytes%s"
          % (name, width, height, len(payload), "" if do_compress else "  raw"))


def add_sound(path, wav, name):
    """One sound, converted to what the card wants and stored raw.

    Never compressed: the compressor is byte-pair run length with no escape for
    a literal, and PCM has almost no runs in it, so compressing a sound costs a
    decompression per play in order to make the bundle larger."""
    items = read_bundle(path)

    if any(existing == name for existing, _, _, _, _, _ in items):
        raise ValueError("%s: %s is already in the bundle" % (path, name))

    channels, payload = read_wav(wav)

    items.append((name, SOUND_RATE, channels, True, RESKIND_SOUND, payload))
    write_bundle(path, items)

    frames = len(payload) // (channels * 2)

    print("  %-34s %-9s %7d bytes  %d ms %s"
          % (name, "sound", len(payload), (frames * 1000) // SOUND_RATE,
             "stereo" if channels == 2 else "mono"))


# -- Resources carried by a module ---------------------------------------------

# Mirrored from kernel/kernel/include/um/modres.h. A field added there has to
# be added here, which is what the size assertion below is for.
FOOTER = "<IIQQ"
FOOTER_SIZE = 24
MODRES_MAGIC = 0x53455242  # BRES
MODRES_VERSION = 1

assert struct.calcsize(FOOTER) == FOOTER_SIZE


def bitmap_item(source, name, do_compress=True, flip=True):
    """One bitmap, ready to go into a bundle."""
    width, height, raw = read_bmp(source, flip)
    payload = compress(raw) if do_compress else raw

    return (name, width, height, not do_compress, RESKIND_BITMAP, payload)


def sound_item(source, name):
    """One sound, converted and ready to go into a bundle."""
    channels, payload = read_wav(source)

    return (name, SOUND_RATE, channels, True, RESKIND_SOUND, payload)


def strip(module):
    """The module without whatever an earlier build attached to it.

    A build that did not relink writes over a file that already carries a
    bundle, so this runs first every time. The footer is trusted only when it
    accounts for the whole tail of the file, since a module whose last bytes
    happen to read as a footer would otherwise be truncated."""
    data = open(module, "rb").read()

    if len(data) < FOOTER_SIZE:
        return data

    magic, _, offset, size = struct.unpack_from(
        FOOTER, data, len(data) - FOOTER_SIZE)

    if magic != MODRES_MAGIC:
        return data

    if offset + size + FOOTER_SIZE != len(data):
        return data

    return data[:offset]


def carried(module):
    """What a module carries, as read_bundle would return it, or None."""
    data = open(module, "rb").read()

    if len(data) < FOOTER_SIZE:
        return None

    magic, version, offset, size = struct.unpack_from(
        FOOTER, data, len(data) - FOOTER_SIZE)

    if magic != MODRES_MAGIC or version != MODRES_VERSION:
        return None

    if offset + size + FOOTER_SIZE != len(data):
        return None

    blob = data[offset:offset + size]
    scratch = module + ".blob"

    # read_bundle takes a path, and the bundle is the same bytes wherever they
    # are. Written out rather than parsed a second way here, so there is one
    # piece of code that knows the layout.
    open(scratch, "wb").write(blob)

    try:
        return read_bundle(scratch)
    finally:
        os.remove(scratch)


def attach(module, items):
    """The bundle, written past the end of the image, and the footer last.

    Padded to sixteen first. Nothing reads the bundle at an alignment, but a
    file whose tail begins at a round number is easier to look at in a dump,
    and the padding costs at most fifteen bytes."""
    body = strip(module)
    body += bytes((-len(body)) % 16)

    offset = len(body)
    blob = bundle_bytes(items)

    open(module, "wb").write(body + blob + struct.pack(
        FOOTER, MODRES_MAGIC, MODRES_VERSION, offset, len(blob)))

    return len(blob)


def show(path):
    items = read_bundle(path)

    for name, width, height, nocompress, kind, payload in items:
        stored = len(payload)
        real = stored if nocompress else len(decompress(payload))

        if kind == RESKIND_SOUND:
            frames = stored // (height * 2) if height else 0
            shape = "sound"
            note = "%d ms, %s" % ((frames * 1000) // width if width else 0,
                                  "stereo" if height == 2 else "mono")
        else:
            shape = "%dx%d" % (width, height)
            note = "raw" if nocompress else ""

        print("  %-34s %-9s %7d -> %-7d %s"
              % (name, shape, real, stored, note))

    print("  %d resources, %d bytes" % (len(items), os.path.getsize(path)))


def main(argv):
    if len(argv) < 3:
        print(__doc__.strip())
        return 1

    path = argv[1]
    action = argv[2]
    rest = argv[3:]

    if action == "--list":
        show(path)
        return 0

    if action == "--add-sound":
        if len(rest) != 2:
            print("multibmpres: --add-sound takes a wav and a name",
                  file=sys.stderr)
            return 2

        add_sound(path, rest[0], rest[1])
        return 0

    if action != "--add":
        print("multibmpres: unknown action %s" % action, file=sys.stderr)
        return 2

    do_compress = True
    flip = True

    while rest and rest[0].startswith("--"):
        if rest[0] == "--no-compress":
            do_compress = False
        elif rest[0] == "--no-flip":
            flip = False
        else:
            print("multibmpres: unknown option %s" % rest[0], file=sys.stderr)
            return 2

        rest = rest[1:]

    if len(rest) != 2:
        print("multibmpres: --add takes a bitmap and a name", file=sys.stderr)
        return 2

    add(path, rest[0], rest[1], do_compress, flip)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
