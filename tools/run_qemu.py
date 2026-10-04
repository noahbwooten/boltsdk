"""Boot BoltOS under QEMU.

    python scripts/run_qemu.py                        headless, 40 seconds
    python scripts/run_qemu.py --interactive          a window, stays up
    python scripts/run_qemu.py --actions "click 40 747; click 90 609"
    python scripts/run_qemu.py --actions "chord ctrl alt f1"      a real chord
    python scripts/run_qemu.py --actions "press 90 200; press 90 200 right; release; release right"
    python scripts/run_qemu.py --actions "type hello; key ret; shot x64/a.png"
    python scripts/run_qemu.py --actions "waitlog shell: \\d+ rows; shot x64/b.png"
    python scripts/run_qemu.py --configuration x64ClientRelease --keep-disk
    python scripts/run_qemu.py --clock 2026-09-26T16:41:00        a fixed start
    python scripts/run_qemu.py --iso x64/x64ClientDebug/boltos.iso  the live CD

Writes the serial log and a screenshot into x64/<configuration>/run.

This replaces run_qemu.ps1, run_qemu_win.ps1 and run_qemu.sh, which were three
descriptions of one thing: a Windows bridge into WSL, the real Windows runner,
and a shell script that built its ESP with mtools. Only the Windows one had
grown the QMP input driving, so driving the machine worked on one platform and
not the other two.

The ESP is handed to QEMU as a directory rather than an image. QEMU's own FAT
emulation builds the volume, so nothing here needs mtools, mkfs or a mount, and
the same code path runs on all three platforms.

Two runners can drive two machines on one host -- two clones of this
repository on one desktop, say -- provided each keeps custody of its own:

    python scripts/run_qemu.py --qmp-port 55656
    python scripts/run_qemu.py --kill-stale

Every port a run listens on follows --qmp-port unless it is given its own:
the monitor takes the port below it, and the host ends of the telnet and
discard forwards the two above it. The usual QMP port keeps the usual
forwards, 2323 and 2399, which is where telnet.ps1 and the network tests look.
Each port is checked before QEMU starts, and a taken one stops the run with
the flag that moves it. A QEMU that cannot take a port exits at once, before
it has opened the serial log, and the quit the runner sends as it finishes
would reach whatever holds the QMP port instead.

A run writes the PID of the QEMU it started to
x64/<configuration>/run/qemu-<qmp port>.pid, and takes the file with it when
it goes. --kill-stale reads what is left over -- a wedged run's machine,
still holding the ports it was given -- and kills those PIDs, each checked
to still be a QEMU before it is fired at, since PIDs get recycled. Killing
by image name instead would take every QEMU on the host down with it,
whoever's they are. The GDB stub takes --gdb-port when :1234 is spoken for,
the same way the QMP and monitor ports move.
"""

import argparse
import json
import os
import platform as _platform
import re
import shutil
import signal
import socket
import struct
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

MONITOR_PORT = 55555
QMP_PORT = 55556

# The host's ends of the two forwarded guest ports: the telnet server's 23,
# and 9, where nothing listens, so the refused path has somewhere to knock.
TELNET_PORT = 2323
DISCARD_PORT = 2399

# QEMU's own default for -s, and what every GDB recipe assumes.
GDB_PORT = 1234

# QEMU's absolute pointing device works in this range whatever the screen is.
ABS_RANGE = 32767

# What the guest boots at, which is what an injected click is measured
# against. BootInitDisplay takes the largest mode up to 1024x768, and QEMU's
# std VGA offers it, so that is what comes up.
SCREEN_W, SCREEN_H = 1024, 768


def kill_stale(configuration, run_dir=None):
    """Kill this folder's leftover QEMUs, by PID, and take nothing else down.

    The pid files a run leaves behind name the machines of this folder and no
    others. Each PID is checked against the process table before it is
    killed: one that is gone, or one that has been recycled onto something
    that is not a QEMU, is left alone and its file cleared.
    """
    run_dir = run_dir or os.path.join(ROOT, "x64", configuration, "run")

    if not os.path.isdir(run_dir):
        print("run_qemu: %s is not there; nothing to kill" % run_dir)
        return 0

    killed = 0

    for name in sorted(os.listdir(run_dir)):
        if not (name.startswith("qemu-") and name.endswith(".pid")):
            continue

        path = os.path.join(run_dir, name)

        try:
            pid = open(path).read().strip()
        except OSError:
            continue

        if _platform.system() == "Windows":
            check = subprocess.run(
                ["tasklist", "/FI", "PID eq %s" % pid, "/FO", "CSV", "/NH"],
                capture_output=True, text=True)
            ours = "qemu" in check.stdout.lower()
        else:
            check = subprocess.run(["ps", "-p", pid, "-o", "comm="],
                                   capture_output=True, text=True)
            ours = check.stdout.strip().startswith("qemu")

        if not ours:
            print("run_qemu: %s held pid %s, which is gone or not a QEMU"
                  % (name, pid))
        else:
            if _platform.system() == "Windows":
                subprocess.run(["taskkill", "/F", "/PID", pid],
                               capture_output=True)
            else:
                try:
                    os.kill(int(pid), signal.SIGKILL)
                except OSError:
                    pass

            killed += 1
            print("run_qemu: killed %s (pid %s)" % (name, pid))

        try:
            os.remove(path)
        except OSError:
            pass

    if not killed:
        print("run_qemu: no strays in %s" % run_dir)

    return 0


def forget_pid(pid_file, pid):
    """Take the custody file back, when the machine it named is down.

    Only while it still names this run's machine. The file is named for the
    port, so a later run on the same port writes the same file, and a run
    whose QEMU --kill-stale took down used to remove the newer run's file as
    it finished, leaving that machine with nothing to find it by."""
    try:
        if open(pid_file).read().strip() == str(pid):
            os.remove(pid_file)
    except OSError:
        pass


def ports_taken(args):
    """The host ports this run would listen on that something already holds,
    as (port, flag) pairs, the flag being the one that moves it."""
    wanted = [(args.qmp_port, "--qmp-port", "127.0.0.1"),
              (args.monitor_port, "--monitor-port", "127.0.0.1")]

    if not args.no_net:
        wanted += [(args.telnet_port, "--telnet-port", "127.0.0.1"),
                   (args.discard_port, "--discard-port", "127.0.0.1")]

    if args.gdb:
        wanted.append((args.gdb_port, "--gdb-port", ""))

    taken = []

    for port, flag, address in wanted:
        with socket.socket() as probe:
            # QEMU asks for fast reuse everywhere but Windows, where the flag
            # means something else and it leaves it off; asked the same way,
            # a port is free here exactly when it is free to QEMU.
            if os.name != "nt":
                probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

            try:
                probe.bind((address, port))
            except OSError:
                taken.append((port, flag))

    return taken


# What the type action sends for a character that is not a letter or a digit:
# the qcode, and whether shift is held for it.
TYPED = {
    " ": ("spc", False), "-": ("minus", False), "=": ("equal", False),
    ".": ("dot", False), ",": ("comma", False), "/": ("slash", False),
    "\\": ("backslash", False), "'": ("apostrophe", False),
    "[": ("bracket_left", False), "]": ("bracket_right", False),
    "`": ("grave_accent", False),
    "!": ("1", True), "@": ("2", True), "#": ("3", True), "$": ("4", True),
    "%": ("5", True), "^": ("6", True), "&": ("7", True), "*": ("8", True),
    "(": ("9", True), ")": ("0", True), "_": ("minus", True),
    "+": ("equal", True), ":": ("semicolon", True), '"': ("apostrophe", True),
    "<": ("comma", True), ">": ("dot", True), "?": ("slash", True),
    "|": ("backslash", True), "~": ("grave_accent", True),
    "{": ("bracket_left", True), "}": ("bracket_right", True),
}

# What a fault looks like in the log. The HAL prints EXCEPTION and the kernel
# prints bugcheck, so looking for one word finds half of them.
FAULTED = re.compile(r"bugcheck|kernel fault|triple fault|EXCEPTION \d+|"
                     r"\[boltos\] halted", re.I)


def host():
    if sys.platform.startswith("linux"):
        return "linux"
    if sys.platform == "darwin":
        return "darwin"
    return "win32"


# -- Finding things ------------------------------------------------------------

QEMU_PLACES = {
    "win32": [r"C:\Program Files\qemu\qemu-system-x86_64.exe",
              r"C:\Program Files (x86)\qemu\qemu-system-x86_64.exe",
              os.path.expandvars(r"%LOCALAPPDATA%\Programs\qemu\qemu-system-x86_64.exe")],
    "linux": ["/usr/bin/qemu-system-x86_64", "/usr/local/bin/qemu-system-x86_64"],
    "darwin": ["/opt/homebrew/bin/qemu-system-x86_64", "/usr/local/bin/qemu-system-x86_64"],
}

# The firmware, which is not part of QEMU on every platform. Debian packages it
# separately as ovmf. Homebrew's qemu formula and the Windows installer both
# carry it as edk2-x86_64-code.fd and edk2-i386-vars.fd, which firmware() also
# looks for beside whichever qemu-system-x86_64 find_qemu() chose.
OVMF_PLACES = [
    "/usr/share/OVMF/OVMF_CODE_4M.fd",
    "/usr/share/OVMF/OVMF_CODE.fd",
    "/usr/share/edk2/x64/OVMF_CODE.4m.fd",
    "/opt/homebrew/share/qemu/edk2-x86_64-code.fd",
    "/usr/local/share/qemu/edk2-x86_64-code.fd",
]

OVMF_VARS_PLACES = [
    "/usr/share/OVMF/OVMF_VARS_4M.fd",
    "/usr/share/OVMF/OVMF_VARS.fd",
    "/usr/share/edk2/x64/OVMF_VARS.4m.fd",
    "/opt/homebrew/share/qemu/edk2-i386-vars.fd",
    "/usr/local/share/qemu/edk2-i386-vars.fd",
]


def find_qemu():
    found = shutil.which("qemu-system-x86_64")

    if found:
        return found

    for path in QEMU_PLACES.get(host(), []):
        if os.path.isfile(path):
            return path

    raise SystemExit(
        "run_qemu: qemu-system-x86_64 not found. See BUILDING.md, or\n"
        "  docs/setup.md in the SDK, for the package to install on this\n"
        "  platform.")


def firmware(qemu):
    """The OVMF code and variable images, as a pair of paths.

    A pair in x64/firmware comes first, then the system's own packages, then
    the pair QEMU's installation carries: the Windows installer keeps it in
    share beside the executable, and a POSIX install in share/qemu under its
    prefix. edk2-x86_64-code.fd and edk2-i386-vars.fd are the same 4M layout as
    OVMF_CODE_4M.fd and OVMF_VARS_4M.fd under QEMU's names. Only when none of
    that is there is the pair copied into x64/firmware out of a WSL Debian."""
    local = os.path.join(ROOT, "x64", "firmware")
    code = os.path.join(local, "OVMF_CODE_4M.fd")
    variables = os.path.join(local, "OVMF_VARS_4M.fd")

    if os.path.isfile(code) and os.path.isfile(variables):
        return code, variables

    for candidate in OVMF_PLACES:
        if os.path.isfile(candidate):
            for store in OVMF_VARS_PLACES:
                if os.path.isfile(store):
                    return candidate, store

    binary = os.path.dirname(os.path.realpath(qemu))

    for share in (os.path.join(binary, "share"),
                  os.path.join(binary, os.pardir, "share", "qemu")):
        candidate = os.path.join(share, "edk2-x86_64-code.fd")
        store = os.path.join(share, "edk2-i386-vars.fd")

        if os.path.isfile(candidate) and os.path.isfile(store):
            return os.path.normpath(candidate), os.path.normpath(store)

    if host() == "win32" and shutil.which("wsl"):
        os.makedirs(local, exist_ok=True)
        print("fetching OVMF firmware from WSL")
        subprocess.run(["wsl", "-d", "Debian", "-u", "root", "--", "cp",
                        "/usr/share/OVMF/OVMF_CODE_4M.fd",
                        "/usr/share/OVMF/OVMF_VARS_4M.fd",
                        subprocess.run(["wsl", "-d", "Debian", "-u", "root", "--",
                                        "wslpath", "-a", local],
                                       capture_output=True, text=True).stdout.strip()],
                       capture_output=True)

        if os.path.isfile(code) and os.path.isfile(variables):
            return code, variables

    raise SystemExit(
        "run_qemu: no OVMF firmware found. See BUILDING.md, or docs/setup.md\n"
        "  in the SDK; on Debian this is the ovmf package, and on macOS and\n"
        "  Windows it ships with QEMU.")


# How many times longer every wait is made when a Windows machine without WHPX
# has to emulate. A GitHub runner took most of two minutes to boot under TCG
# where an accelerated boot takes twenty seconds, and verify.py's 30 seconds of
# --boot-wait becomes the 150 that CI gives its own emulated boot. It is sized
# for a slow host: a four-core desktop reached the shell in 8 seconds under TCG
# against 6 under WHPX.
TCG_SLOWDOWN = 5


def acceleration():
    """The accelerator, and whether the interrupt controller has to stay in
    userspace.

    WHPX is the only one that coexists with Hyper-V, which is what a Windows
    machine with WSL is already running. It cannot emulate the interrupt
    controller, and the PS/2 driver depends on the PICs, so that is turned off
    alongside it. It is an optional Windows feature, so main() asks
    whpx_starts() before relying on it. On Apple silicon an x86-64 guest is
    emulated whatever is installed, so there is nothing to ask for."""
    if host() == "win32":
        return "whpx", True

    if host() == "linux" and os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK):
        return "kvm", False

    if host() == "darwin" and _platform.machine() in ("x86_64", "amd64"):
        return "hvf", False

    return "tcg", False


def whpx_starts(qemu):
    """Whether QEMU can bring WHPX up on this machine.

    Without the Windows Hypervisor Platform feature, QEMU asked for WHPX stops
    at once with "failed to initialize whpx" and writes no serial log. So a
    q35 machine is started first under -S, which holds it before the first
    instruction: it answers QMP only once the accelerator is up, and is told
    to quit as soon as it does. That costs about half a second where WHPX is
    there, and two where it is not, most of it Windows refusing a connection.

    -machine none would be quicker, but QEMU 11.1 asserts on it with WHPX
    before it gets as far as looking for a hypervisor. A failure that is not
    about WHPX counts as WHPX starting, so the real run reports it rather than
    TCG hiding it."""
    # A port nothing is listening on rather than QMP_PORT, where a run that is
    # still up would be the one told to quit.
    with socket.socket() as spare:
        spare.bind(("127.0.0.1", 0))
        port = spare.getsockname()[1]

    probe = subprocess.Popen(
        [qemu, "-machine", "q35", "-accel", "whpx,kernel-irqchip=off", "-S",
         "-display", "none", "-nodefaults",
         "-qmp", "tcp:127.0.0.1:%d,server,nowait" % port],
        stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        text=True, errors="replace")

    errors = ""
    deadline = time.time() + 20

    try:
        while probe.poll() is None and time.time() < deadline:
            try:
                qmp = Qmp(port)
                qmp.command("quit")
                qmp.close()
                break
            except OSError:
                time.sleep(0.1)

        errors = probe.communicate(timeout=10)[1]
    except subprocess.TimeoutExpired:
        pass
    finally:
        if probe.poll() is None:
            probe.kill()
            probe.communicate()

    # The complaint that it has no performance counters to offer mentions WHPX
    # too, but as a warning, and comes from one that did start.
    return not any("whpx" in line.lower() and "warning:" not in line
                   for line in errors.splitlines())


# -- The BoltOS volume ---------------------------------------------------------

DISK_BYTES = 64 * 1024 * 1024


# Where the guest's audio goes, per platform. QEMU has to be told: it will not
# pick one, and the wrong name is a fatal argument rather than a fallback.
AUDIO_HOSTS = {
    "win32": "dsound",
    "linux": "pipewire",
    "darwin": "coreaudio",
}


def audio_backend(args, out_dir):
    """The -audiodev argument, as one string.

    "none" still models the device to the clock, so the card fetches its buffer
    at the sample rate and everything downstream behaves identically. It is the
    default for an automated run because opening a host device on a machine
    that has none is a warning nobody reads, or a failure to start.

    "wav" is the one worth knowing about: QEMU writes what the guest played to
    a file, so a mix can be checked sample by sample on the host rather than by
    listening to it. It writes only while the device is producing, so silence
    between two sounds is not in the file and a timestamp in it is not a
    timestamp in the run.

    QEMU complains that it cannot open `ac97.pi' and `ac97.mc' under any
    backend with no capture side, which is both of these. Those are the card's
    record streams. Nothing in BoltOS records, and the warning cannot be turned
    off from here."""
    choice = args.audio_out or ("host" if args.interactive else "none")

    if choice == "wav":
        path = os.path.join(out_dir, "audio.wav")
        print("audio: writing %s" % path)

        # QEMU resamples to whatever the backend asks for, and asking for what
        # the guest already produces means the file holds its samples rather
        # than a conversion of them.
        return ("wav,id=snd0,path=%s,out.frequency=48000,out.channels=2"
                % path.replace("\\", "/"))

    if choice == "host":
        backend = AUDIO_HOSTS.get(host())

        if not backend:
            print("audio: no host backend known for %s; running silent" % host())
            return "none,id=snd0"

        print("audio: %s" % backend)
        return "%s,id=snd0" % backend

    return "none,id=snd0"


def finish_wav(path):
    """Write the two length fields QEMU leaves at zero.

    It fills them in as it closes the file, and it is not around to do that
    when the run ends by being told to quit. A reader that trusts the header
    then finds a file of no length, so the sizes are put back from what is
    actually there."""
    if not os.path.exists(path) or os.path.getsize(path) <= 44:
        return

    size = os.path.getsize(path)

    with open(path, "r+b") as handle:
        handle.seek(4)
        handle.write(struct.pack("<I", size - 8))
        handle.seek(40)
        handle.write(struct.pack("<I", size - 44))

    print("audio: %s (%.2f s)" % (path, (size - 44) / (48000.0 * 4)))


def make_volume(path, fs_image, keep):
    """The disk the machine actually boots.

    The only thing the machine boots from: the EFI binary carries no
    filesystem of its own any more. It is refreshed when the build is newer,
    because a rebuilt kernel that silently never ran is a bad hour and nothing
    on the screen says so.

    Sized well beyond fs.bin rather than exactly to it: the filesystem appends,
    so a disk cut to the image's current length has no room for the first write
    and the transfer runs off the end of the device."""
    if not os.path.exists(fs_image):
        print("warning: no filesystem image at %s" % fs_image)
        return

    stale = os.path.exists(path) and \
        os.path.getmtime(fs_image) > os.path.getmtime(path)

    if stale and keep:
        print("warning: %s is newer than the disk; booting the older one"
              % os.path.basename(fs_image))
        stale = False

    if stale:
        print("filesystem image is newer than the disk; rebuilding it")
        os.remove(path)

    if os.path.exists(path):
        return

    payload = open(fs_image, "rb").read()
    size = max(DISK_BYTES, (len(payload) + 511) // 512 * 512)

    with open(path, "wb") as out:
        out.write(payload)
        out.write(b"\0" * (size - len(payload)))

    print("created %s (%d bytes, fs.bin is %d)" % (path, size, len(payload)))


# -- Driving the machine -------------------------------------------------------

class Qmp(object):
    """One QMP connection.

    The human monitor's mouse_move only ever emits relative motion, which an
    absolute pointing device does not accept, so positioning the pointer for a
    test needs input-send-event and that is QMP only. qmp_capabilities is
    mandatory: the connection ignores every command until it has been sent."""

    def __init__(self, port=QMP_PORT, timeout=5.0):
        self.socket = socket.create_connection(("127.0.0.1", port), timeout)
        self.stream = self.socket.makefile("rwb")
        self.stream.readline()                      # the greeting
        self.command("qmp_capabilities")

    def command(self, name, **arguments):
        message = {"execute": name}

        if arguments:
            message["arguments"] = arguments

        self.stream.write((json.dumps(message) + "\r\n").encode())
        self.stream.flush()

        while True:
            line = self.stream.readline()

            if not line:
                return None

            reply = json.loads(line.decode())

            if "return" in reply or "error" in reply:
                return reply

    def close(self):
        try:
            self.stream.close()
            self.socket.close()
        except OSError:
            pass

    # -- input --

    def move(self, x, y):
        self.command("input-send-event", events=[
            {"type": "abs", "data": {"axis": "x",
                                     "value": x * ABS_RANGE // SCREEN_W}},
            {"type": "abs", "data": {"axis": "y",
                                     "value": y * ABS_RANGE // SCREEN_H}}])

    def button(self, button, down):
        """One button, down or up, left where it is. What a click is made of,
        for a press that has to be held: a screen shown while the button is
        down, or two buttons down together."""
        self.command("input-send-event", events=[
            {"type": "btn", "data": {"down": down, "button": button}}])

    def click(self, x, y, button="left"):
        self.move(x, y)
        time.sleep(0.2)
        self.command("input-send-event", events=[
            {"type": "btn", "data": {"down": True, "button": button}}])
        time.sleep(0.12)
        self.command("input-send-event", events=[
            {"type": "btn", "data": {"down": False, "button": button}}])

    def dblclick(self, x, y):
        """Two presses close enough together to be one gesture.

        The desktop opens an entry on a double click and the window it accepts
        is four hundred milliseconds, which the gap between two separate click
        actions is comfortably outside of."""
        self.move(x, y)
        time.sleep(0.2)

        for _ in range(2):
            self.command("input-send-event", events=[
                {"type": "btn", "data": {"down": True, "button": "left"}}])
            time.sleep(0.06)
            self.command("input-send-event", events=[
                {"type": "btn", "data": {"down": False, "button": "left"}}])
            time.sleep(0.06)

    def drag(self, x1, y1, x2, y2):
        self.move(x1, y1)
        time.sleep(0.2)
        self.command("input-send-event", events=[
            {"type": "btn", "data": {"down": True, "button": "left"}}])

        # Stepped rather than jumped. A window drag is followed frame by frame,
        # and a single move from one corner to the other is one event the guest
        # may never see between polls.
        for step in range(1, 9):
            self.move(x1 + (x2 - x1) * step // 8, y1 + (y2 - y1) * step // 8)
            time.sleep(0.06)

        self.command("input-send-event", events=[
            {"type": "btn", "data": {"down": False, "button": "left"}}])

    def wheel(self, notches):
        button = "wheel-up" if notches > 0 else "wheel-down"

        for _ in range(abs(notches)):
            self.command("input-send-event", events=[
                {"type": "btn", "data": {"down": True, "button": button}}])
            self.command("input-send-event", events=[
                {"type": "btn", "data": {"down": False, "button": button}}])
            time.sleep(0.08)

    def key(self, name):
        # Held for a moment: the window manager finds keys by comparing one
        # frame's keyboard with the last, so a press released inside a single
        # frame was never there.
        self.command("input-send-event", events=[
            {"type": "key", "data": {"down": True,
                                     "key": {"type": "qcode", "data": name}}}])
        time.sleep(0.05)
        self.command("input-send-event", events=[
            {"type": "key", "data": {"down": False,
                                     "key": {"type": "qcode", "data": name}}}])

    def hold(self, name, seconds):
        """One key held down for a while, which is what repeat is made of."""
        self.command("input-send-event", events=[
            {"type": "key", "data": {"down": True,
                                     "key": {"type": "qcode", "data": name}}}])
        time.sleep(seconds)
        self.command("input-send-event", events=[
            {"type": "key", "data": {"down": False,
                                     "key": {"type": "qcode", "data": name}}}])

    def chord(self, names):
        """Hold several keys down together, then let them all go.

        key() presses and releases one key, so a combination sent that way is
        three presses in sequence and never a chord. The crash screens and the
        deliberate-fault keys both want the real thing.
        """
        down = [{"type": "key", "data": {"down": True,
                                         "key": {"type": "qcode", "data": n}}}
                for n in names]
        up = [{"type": "key", "data": {"down": False,
                                       "key": {"type": "qcode", "data": n}}}
              for n in reversed(names)]

        self.command("input-send-event", events=down)
        time.sleep(0.3)
        self.command("input-send-event", events=up)

    def type(self, text):
        """A string, one key at a time, shifted where the character needs it.

        Spaced out rather than sent at once: the guest samples the keyboard
        once a frame, and two keys inside one frame are one key."""
        for character in text:
            code, shifted = TYPED.get(character, (None, False))

            if code is None and character.isalnum():
                code, shifted = character.lower(), character.isupper()

            if code is None:
                print("  cannot type %r" % character)
                continue

            keys = (["shift"] if shifted else []) + [code]

            self.command("input-send-event", events=[
                {"type": "key", "data": {"down": True,
                                         "key": {"type": "qcode", "data": k}}}
                for k in keys])
            time.sleep(0.05)
            self.command("input-send-event", events=[
                {"type": "key", "data": {"down": False,
                                         "key": {"type": "qcode", "data": k}}}
                for k in reversed(keys)])
            time.sleep(0.05)

    def screenshot(self, path):
        # format was added to screendump in QEMU 7.1. Without it the output is
        # PPM, which nothing on a Windows desktop opens, so the older spelling
        # is only a fallback.
        reply = self.command("screendump", filename=path, format="png")

        if reply and "error" in reply:
            reply = self.command("screendump", filename=path)

        return not (reply and "error" in reply)


def guest_wants_reset(log):
    """Whether the serial log ends with the guest asking to be reset.

    Every restart goes through HalUpdatePowerState(1), which logs
    "power: restart"; a shutdown logs "power: shutdown". Reading the log's
    tail rather than QEMU's exit code, because a quit, a closed window and a
    reset all leave QEMU with the same status. Nothing there means no.
    """
    try:
        with open(log, "rb") as f:
            tail = f.read()[-16384:]
    except OSError:
        return False

    restart = tail.rfind(b"[boltos] power: restart")
    shutdown = tail.rfind(b"[boltos] power: shutdown")
    boot = tail.rfind(b"[boltos] uefi hal starting")

    # A boot banner after the marker is a machine that has already come back.
    return restart != -1 and restart > shutdown and restart > boot


def wait_for_log(log, pattern, timeout=120.0):
    """Until the serial log says something, rather than for a guessed time.

    A boot stage is photographed when the machine reports reaching it, which
    holds on a slower host or under emulation where a fixed wait does not."""
    expression = re.compile(pattern)
    deadline = time.time() + timeout

    while time.time() < deadline:
        if log and os.path.exists(log):
            text = open(log, encoding="utf-8", errors="replace").read()

            if expression.search(text):
                return True

        time.sleep(0.1)

    return False


def inject(qmp, actions, log=None):
    """Run one semicolon-separated action list against the guest.

    shot takes a picture there and then, so one run can photograph several
    screens; a relative path is under the repository root. type sends the rest
    of the step as keystrokes, spaces included, and waitlog waits for the rest
    of the step, a regular expression, to appear in the serial log."""
    for raw in actions.split(";"):
        step = raw.strip()

        if not step:
            continue

        parts = step.split()
        name = parts[0]

        try:
            if name == "type" and len(parts) >= 2:
                qmp.type(step[len("type"):].strip())
            elif name == "waitlog" and len(parts) >= 2:
                if not wait_for_log(log, step[len("waitlog"):].strip()):
                    print("  never logged: %s" % step)
            elif name == "shot" and len(parts) == 2:
                path = os.path.join(ROOT, parts[1])
                os.makedirs(os.path.dirname(path), exist_ok=True)

                if not qmp.screenshot(path):
                    print("  screenshot failed: %s" % parts[1])
            elif name == "click" and len(parts) == 3:
                qmp.click(int(parts[1]), int(parts[2]))
            elif name == "rclick" and len(parts) == 3:
                qmp.click(int(parts[1]), int(parts[2]), "right")
            elif name == "dblclick" and len(parts) == 3:
                qmp.dblclick(int(parts[1]), int(parts[2]))
            elif name == "press" and len(parts) in (3, 4):
                qmp.move(int(parts[1]), int(parts[2]))
                time.sleep(0.2)
                qmp.button(parts[3] if len(parts) == 4 else "left", True)
            elif name == "release" and len(parts) in (1, 2):
                qmp.button(parts[1] if len(parts) == 2 else "left", False)
            elif name == "abs" and len(parts) == 3:
                qmp.move(int(parts[1]), int(parts[2]))
            elif name == "drag" and len(parts) == 5:
                qmp.drag(*[int(p) for p in parts[1:5]])
            elif name == "wheel" and len(parts) == 2:
                qmp.wheel(int(parts[1]))
            elif name == "key" and len(parts) == 2:
                qmp.key(parts[1])
            elif name == "chord" and len(parts) >= 2:
                qmp.chord(parts[1:])
            elif name == "hold" and len(parts) == 3:
                qmp.hold(parts[1], float(parts[2]))
            elif name == "wait" and len(parts) == 2:
                time.sleep(float(parts[1]))
            else:
                print("  unknown action: %s" % step)
                continue
        except (OSError, ValueError) as problem:
            print("  action failed (%s): %s" % (step, problem))
            continue

        print("  %s" % step)
        time.sleep(0.4)


# -- Going ---------------------------------------------------------------------

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--configuration", default="x64ClientDebug",
                        choices=["x64ClientDebug", "x64ClientRelease"])
    parser.add_argument("--seconds", type=int, default=40,
                        help="how long to let the machine run")
    parser.add_argument("--boot-wait", type=int, default=25,
                        help="how long to let it boot before injecting anything")
    parser.add_argument("--actions", default="",
                        help='semicolon separated, e.g. "click 40 747; wheel -3"')
    parser.add_argument("--memory", type=int, default=512)
    parser.add_argument("--interactive", action="store_true",
                        help="open a window and stay up")
    parser.add_argument("--keep-disk", action="store_true",
                        help="boot the previous disk even if the build is newer")
    parser.add_argument("--no-disk", action="store_true",
                        help="attach no volume, so the boot fails at the loader")
    parser.add_argument("--iso", default=None, metavar="PATH",
                        help="boot this ISO as the CD; the ESP rig is left out "
                             "so the CD is what the firmware boots")
    parser.add_argument("--disk-bus", default="ahci",
                        choices=["ahci", "nvme", "both"],
                        help="which controller the BoltOS volume hangs off; "
                             "nvme is the only way to exercise that driver")
    parser.add_argument("--no-cd", action="store_true")
    parser.add_argument("--no-net", action="store_true")
    parser.add_argument("--no-audio", action="store_true")
    parser.add_argument("--audio-out", default="",
                        choices=["", "none", "host", "wav"],
                        help="where the guest's audio goes: none is silent, "
                             "host plays it, wav writes audio.wav beside the "
                             "log. Defaults to host with --interactive and "
                             "none without")
    parser.add_argument("--capture", action="store_true",
                        help="record every frame to a pcap beside the log")
    parser.add_argument("--gdb", action="store_true",
                        help="open the GDB stub and keep running")
    parser.add_argument("--gdb-port", type=int, default=GDB_PORT,
                        help="the port the GDB stub listens on; give a "
                             "second run on this host its own, like "
                             "--qmp-port")
    parser.add_argument("--gdb-wait", action="store_true",
                        help="open the stub and freeze until a debugger attaches")
    parser.add_argument("--kill-stale", action="store_true",
                        help="kill this folder's leftover QEMUs by PID, from "
                             "the pid files past runs wrote, and exit; never "
                             "kills by image name, so another folder's "
                             "machines are untouched")
    parser.add_argument("--clock", metavar="UTC",
                        help="start the guest clock at this UTC time, e.g. "
                             "2026-09-26T16:41:00, rather than the host's")
    parser.add_argument("--qmp-port", type=int, default=QMP_PORT,
                        help="the QMP port; give a second run on this host "
                             "its own, so the two do not hold each other's "
                             "machines. The monitor and forwarded ports "
                             "follow it unless given their own")
    parser.add_argument("--monitor-port", type=int, default=None,
                        help="the human monitor's port; the one below "
                             "--qmp-port unless given")
    parser.add_argument("--telnet-port", type=int, default=None,
                        help="the host port forwarded to the guest's telnet "
                             "server; %d at the usual --qmp-port, the one "
                             "above it otherwise" % TELNET_PORT)
    parser.add_argument("--discard-port", type=int, default=None,
                        help="the host port forwarded to the guest's port 9, "
                             "where nothing listens; %d at the usual "
                             "--qmp-port, two above it otherwise"
                             % DISCARD_PORT)
    parser.add_argument("--efi", metavar="PATH",
                        help="boot this loader rather than the configuration's "
                             "bootx64.efi")
    parser.add_argument("--fs-image", metavar="PATH",
                        help="make the disk from this volume rather than the "
                             "configuration's fs.bin")
    parser.add_argument("--run-dir", metavar="PATH",
                        help="keep the disk, the log and the screenshot here "
                             "rather than in the configuration's run folder")
    parser.add_argument("--accel", choices=["whpx", "kvm", "hvf", "tcg"],
                        help="use this accelerator rather than the host's "
                             "usual one; tcg emulates, slowly enough that "
                             "the boot screens can be watched")
    args = parser.parse_args(argv)

    if args.kill_stale:
        return kill_stale(args.configuration, args.run_dir)

    # A debugging session runs until it is finished with, so neither form is
    # given a deadline. --gdb-wait also implies the stub.
    if args.gdb_wait:
        args.gdb = True

    # One number moves a runner: every port follows --qmp-port unless given.
    # The usual one keeps the usual ports, where telnet.ps1 and the tests look.
    moved = args.qmp_port != QMP_PORT

    if args.monitor_port is None:
        args.monitor_port = args.qmp_port - 1 if moved else MONITOR_PORT
    if args.telnet_port is None:
        args.telnet_port = args.qmp_port + 1 if moved else TELNET_PORT
    if args.discard_port is None:
        args.discard_port = args.qmp_port + 2 if moved else DISCARD_PORT

    # Before anything is started. A QEMU that cannot take a port exits before
    # it opens the serial log, which reads as a guest that died unseen, and
    # the quit sent as the run finishes would go to whoever has the port.
    taken = ports_taken(args)

    if taken:
        for port, flag in taken:
            print("run_qemu: port %d is held by something else on this host; "
                  "%s moves it" % (port, flag))

        print("run_qemu: a second runner on this host needs its own "
              "--qmp-port, which moves every port but the GDB stub's")
        return 1

    # A configuration's own files, or the three paths given instead. The SDK
    # runs this outside the tree, on the loader and the volume it ships, so
    # nothing below may assume the x64 folder exists.
    built = os.path.join(ROOT, "x64", args.configuration)
    out_dir = os.path.abspath(args.run_dir or os.path.join(built, "run"))
    efi = os.path.abspath(args.efi or os.path.join(built, "bootx64.efi"))
    fs_image = os.path.abspath(args.fs_image or os.path.join(built, "fs.bin"))

    if not os.path.exists(efi):
        raise SystemExit("run_qemu: not built: %s" % efi)

    if args.iso and not os.path.exists(args.iso):
        raise SystemExit("run_qemu: no such ISO: %s" % args.iso)

    os.makedirs(out_dir, exist_ok=True)

    qemu = find_qemu()
    code, vars_master = firmware(qemu)
    accel, no_irqchip = acceleration()

    if args.accel:
        accel, no_irqchip = args.accel, args.accel == "whpx"
    elif accel == "whpx" and not whpx_starts(qemu):
        # Every wait was sized for WHPX, whether it came from the defaults or
        # from a caller like verify.py, so each is stretched to match.
        accel, no_irqchip = "tcg", False
        args.seconds *= TCG_SLOWDOWN
        args.boot_wait *= TCG_SLOWDOWN
        print('accel: WHPX is not available, so falling back to TCG with every '
              'wait %dx longer; turn on the Windows feature "Windows Hypervisor '
              'Platform" to make it fast' % TCG_SLOWDOWN)

    # The ESP, as a directory QEMU turns into a FAT volume.
    esp = os.path.join(out_dir, "esp", "EFI", "BOOT")
    os.makedirs(esp, exist_ok=True)
    shutil.copyfile(efi, os.path.join(esp, "BOOTX64.EFI"))

    # A fresh variable store each run. Reusing one lets a failed boot leave
    # state that quietly changes the next one.
    variables = os.path.join(out_dir, "OVMF_VARS.fd")
    shutil.copyfile(vars_master, variables)

    log = os.path.join(out_dir, "serial.log")
    shot = os.path.join(out_dir, "screen.png")

    for stale in (log, shot):
        if os.path.exists(stale):
            os.remove(stale)

    # The same volume on whichever bus this run is testing. Two images rather
    # than one shared file, because --disk-bus both attaches them at the same
    # time and a single image behind two controllers is a way to corrupt it
    # rather than a way to save a copy.
    disk = os.path.join(out_dir, "bolt-disk.img")
    nvme = os.path.join(out_dir, "bolt-nvme.img")

    on_ahci = args.disk_bus in ("ahci", "both")
    on_nvme = args.disk_bus in ("nvme", "both")

    if args.no_disk:
        print("no volume attached; the machine will stop at the loader")
    else:
        if on_ahci:
            make_volume(disk, fs_image, args.keep_disk)
        if on_nvme:
            make_volume(nvme, fs_image, args.keep_disk)

    machine = "q35,vmport=on"

    if not args.no_audio:
        machine += ",pcspk-audiodev=snd0"

    command = [
        qemu,
        "-machine", machine,
        "-vga", "std",
        "-device", "qemu-xhci,id=xhci",
        "-device", "usb-kbd,bus=xhci.0",
        "-m", str(args.memory),
        "-drive", "if=pflash,format=raw,unit=0,readonly=on,file=%s" % code,
        "-drive", "if=pflash,format=raw,unit=1,file=%s" % variables,
    ]

    # The ESP, as a directory QEMU turns into a FAT volume. Not when this run
    # boots an ISO: there the CD's own boot record is the whole point.
    if not args.iso:
        command += ["-drive",
                    "file=fat:rw:%s,format=raw" % os.path.join(out_dir, "esp")]

    command += [
        "-serial", "file:%s" % log,
        "-monitor", "tcp:127.0.0.1:%d,server,nowait" % args.monitor_port,
        "-qmp", "tcp:127.0.0.1:%d,server,nowait" % args.qmp_port,
    ]

    if not args.no_disk and on_ahci and os.path.exists(disk):
        command += ["-drive", "file=%s,format=raw,if=none,id=bolt0" % disk,
                    "-device", "ide-hd,drive=bolt0,bus=ide.1"]

    if not args.no_disk and on_nvme and os.path.exists(nvme):
        command += ["-drive", "file=%s,format=raw,if=none,id=boltnvme" % nvme,
                    "-device", "nvme,drive=boltnvme,serial=BOLTOS0001"]

    if args.iso:
        # The medium being tested is the boot path, so the ESP rig stays out:
        # a FAT drive carrying BOOTX64.EFI would be a second, easier answer,
        # and the firmware would take it before it ever reached the CD.
        command += ["-drive", "file=%s,if=none,id=cd0,media=cdrom,readonly=on"
                    % args.iso,
                    "-device", "ide-cd,drive=cd0,bus=ide.2"]
        print("cd:    %s" % args.iso)
    elif not args.no_cd:
        # An empty tray is still a drive, and the code path for one is worth
        # having exercised.
        command += ["-device", "ide-cd,bus=ide.2"]

    if args.no_net:
        command += ["-net", "none"]
    else:
        # Port 23 forwarded so the telnet server is reachable, and port 9 so the
        # connection-refused path has somewhere to knock that nobody answers.
        command += [
            "-netdev",
            "user,id=net0,hostfwd=tcp:127.0.0.1:%d-:23,hostfwd=tcp:127.0.0.1:%d-:9"
            % (args.telnet_port, args.discard_port),
            "-device", "rtl8139,netdev=net0"]

        if args.capture:
            pcap = os.path.join(out_dir, "boltos.pcap")

            if os.path.exists(pcap):
                os.remove(pcap)

            command += ["-object",
                        "filter-dump,id=netdump,netdev=net0,file=%s" % pcap]
            print("capture: %s" % pcap)

    if not args.no_audio:
        command += ["-audiodev", audio_backend(args, out_dir),
                    "-device", "AC97,audiodev=snd0"]

    if accel != "tcg":
        command += ["-accel", accel + (",kernel-irqchip=off" if no_irqchip else "")]

    # WHPX cannot reset a partition it is running. The guest's reset request
    # reaches QEMU, and the context-set that should put the virtual processor
    # back at the firmware's reset vector fails (seen here as WHPX: Failed to
    # set virtual processor context, hr=c0350015, HV_STATUS_INVALID_PARTITION
    # _STATE); the processor ends up at 0xfff0 with its old long-mode state and
    # double-faults, the machine wedges with the old frame still on screen, and
    # QEMU itself is gone not long after. -no-reboot turns a guest reset into a
    # clean exit instead; the interactive run below boots the machine again,
    # which is what a restart is meant to do.
    if accel == "whpx":
        command.append("-no-reboot")

    if args.clock:
        command += ["-rtc", "base=" + args.clock]

    if args.gdb:
        command += ["-gdb", "tcp::%d" % args.gdb_port]

        if args.gdb_wait:
            command.append("-S")

    # A window needs something to open it into. On a headless Linux box, and on
    # WSL without WSLg or an X server, QEMU fails at startup with a display
    # initialisation error that says nothing about the real problem. The old
    # runner warned and carried on headless, which is the right behaviour: the
    # serial log is what most runs are after anyway.
    if args.interactive and host() == "linux" \
            and not os.environ.get("DISPLAY") \
            and not os.environ.get("WAYLAND_DISPLAY"):
        print("warning: no DISPLAY or WAYLAND_DISPLAY; running headless instead")
        print("         the serial log and the screenshot are still written")
        args.interactive = False

    if not args.interactive:
        command += ["-display", "none"]

    print("qemu:  %s" % qemu)
    print("accel: %s" % accel)

    if not args.no_net:
        print("net:   telnet on 127.0.0.1:%d" % args.telnet_port)

    process = subprocess.Popen(command)

    # Custody: which QEMU is this run's, said in a way that outlives the
    # script. --kill-stale reads these files; killing by image name would
    # take another folder's machines down with ours.
    pid_file = os.path.join(out_dir, "qemu-%d.pid" % args.qmp_port)

    with open(pid_file, "w") as out:
        out.write(str(process.pid))

    if args.gdb:
        print()
        print("GDB stub on 127.0.0.1:%d%s" % (args.gdb_port,
              ", frozen until something attaches" if args.gdb_wait else ""))
        print("  see the debugging section of BUILDING.md for the attach recipe")

    # A debugging session and a window both run until they are done with.
    # With -no-reboot set above, a guest reset is QEMU leaving: boot the
    # machine again, which is what the guest asked for. The log tail tells a
    # reset from a shutdown, a quit and a closed window.
    if args.interactive or args.gdb:
        while True:
            try:
                process.wait()
            finally:
                forget_pid(pid_file, process.pid)

            if not guest_wants_reset(log):
                return 0

            print("run_qemu: the guest reset; WHPX cannot reset a running"
                  " partition, so booting the machine again")
            process = subprocess.Popen(command)

            with open(pid_file, "w") as out:
                out.write(str(process.pid))

    try:
        deadline = time.time() + args.seconds

        if args.actions:
            time.sleep(min(args.boot_wait, args.seconds))
            print("injecting: %s" % args.actions)

            # Retried, because with a short --boot-wait QEMU may not be
            # listening yet, and a refused connection skipped every action.
            qmp, problem = None, None

            for _ in range(50):
                try:
                    qmp = Qmp(args.qmp_port)
                    break
                except OSError as refused:
                    problem = refused
                    time.sleep(0.2)

            if qmp:
                try:
                    inject(qmp, args.actions, log)
                    qmp.close()
                except OSError as lost:
                    print("  qmp lost: %s" % lost)
            else:
                print("  qmp unreachable: %s" % problem)

        # Two seconds of settling before the picture, so the last action has
        # been drawn rather than caught halfway.
        #
        # In slices, and over as soon as this run's QEMU is: one killed with
        # --kill-stale left its run asleep for the rest of --seconds, and the
        # run then photographed whatever answered on its port by then, which
        # was the next run's machine.
        remaining = max(0.0, deadline - time.time())
        until = time.time() + (min(remaining, 2.0) if args.actions else remaining)

        while time.time() < until and process.poll() is None:
            time.sleep(min(0.5, max(0.0, until - time.time())))

        if process.poll() is None:
            try:
                qmp = Qmp(args.qmp_port)

                if qmp.screenshot(shot):
                    print("screenshot: %s" % shot)

                qmp.close()
            except OSError as problem:
                print("  qmp unreachable for the screenshot: %s" % problem)
    finally:
        # Ask before insisting. A killed QEMU leaves the sizes in a captured
        # wav header at zero, since it patches them as it closes the file, and
        # a reader that trusts a header will call that file empty.
        #
        # Only while this run's QEMU is up: one that has exited gave its port
        # back, and whatever answers on it now is somebody else's machine.
        if process.poll() is None:
            try:
                qmp = Qmp(args.qmp_port)
                qmp.command("quit")
                qmp.close()
            except OSError:
                pass
        else:
            print("  qemu had already exited, with code %d"
                  % process.returncode)

        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.terminate()

            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()

        forget_pid(pid_file, process.pid)

    if not args.no_audio and (args.audio_out == "wav"):
        finish_wav(os.path.join(out_dir, "audio.wav"))

    print("serial log: %s" % log)

    if os.path.exists(log):
        text = open(log, encoding="utf-8", errors="replace").read()
        bad = [l for l in text.splitlines()
               if FAULTED.search(l)]

        if bad:
            print()
            print("FAULTS in the serial log:")

            for line in bad[:10]:
                print("  " + line)

            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
