"""Finding clang and lld.

BoltOS is built by clang and linked by lld on every platform, targeting
x86_64-pc-windows-msvc from whatever host is running the build. Nothing above
the HAL calls a Windows API and no import library is passed to any link, so the
target is a PE/COFF format decision and not a dependency on Windows: a Debian
or macOS host needs clang and lld and nothing else.

The target is always named explicitly. clang-cl defaults to the MSVC target on
a Windows host and would have to be told on any other, so saying it every time
means one command line rather than two.
"""

import glob
import os
import re
import shutil
import sys

TARGET = "x86_64-pc-windows-msvc"

# A toolchain shipped beside these scripts, which is tried before anything on
# the machine: toolchain/bin next to the folder this file is in. The SDK keeps
# clang and lld there when they are packaged with it, so an SDK that brings its
# own compiler builds with that one and not with whatever else is installed.
# In the BoltOS tree there is no such folder and nothing changes.
BUNDLED = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                       "toolchain", "bin")

# Where a toolchain is found when it is not on PATH. BOLT_LLVM is tried first
# so a machine with several can pick one without reordering PATH.
SEARCH = {
    "win32": [
        r"C:\Program Files\LLVM\bin",
        os.path.expanduser(r"~\llvm\bin"),
        os.path.expanduser(r"~\scoop\apps\llvm\current\bin"),
    ],
    # /usr/lib/llvm-* is globbed and sorted rather than listed. Debian names the
    # directory after the major version, and a list would have to be edited
    # every time the distribution moves forward.
    "linux": [
        "/usr/bin", "/usr/local/bin",
        os.path.expanduser("~/llvm/bin"),
    ],
    # Homebrew splits lld out of llvm into a formula of its own, so lld-link is
    # found under a different prefix from clang.
    "darwin": [
        "/opt/homebrew/opt/llvm/bin",     # Apple silicon
        "/opt/homebrew/opt/lld/bin",
        "/usr/local/opt/llvm/bin",        # Intel
        "/usr/local/opt/lld/bin",
        "/opt/local/libexec/llvm/bin",    # MacPorts
        os.path.expanduser("~/llvm/bin"),
    ],
}


def platform():
    if sys.platform.startswith("linux"):
        return "linux"
    if sys.platform == "darwin":
        return "darwin"
    return "win32"


def directories():
    found = []

    if os.environ.get("BOLT_LLVM"):
        found.append(os.path.join(os.environ["BOLT_LLVM"], "bin"))

    found.extend(SEARCH.get(platform(), []))

    if platform() == "linux":
        # Newest first, so a machine with several picks the one most likely to
        # have every tool. Sorted by the version number rather than the name,
        # which would put llvm-9 after llvm-19.
        versioned = glob.glob("/usr/lib/llvm-*/bin")
        versioned.sort(key=lambda p: int(re.search(r"llvm-(\d+)", p).group(1)),
                       reverse=True)
        found.extend(versioned)

    return found


def find(name):
    """One tool, by name.

    PATH first, so a toolchain the shell already knows about is the one used.
    Apple's clang is deliberately not a fallback on macOS: it is a real clang
    but ships no lld-link, and a build that compiles and cannot link is a worse
    failure than one that says what is missing. Nor is it taken from PATH:
    /usr/bin is always on PATH on a Mac and Homebrew's llvm never is, so a PATH
    lookup for clang would return Apple's ahead of the clang-cl beside it."""
    exe = name + (".exe" if platform() == "win32" else "")

    bundled = os.path.join(BUNDLED, exe)
    if os.path.isfile(bundled):
        return bundled

    on_path = shutil.which(name)
    if on_path and not (platform() == "darwin" and
                        os.path.dirname(on_path) == "/usr/bin"):
        return on_path

    for where in directories():
        candidate = os.path.join(where, exe)
        if os.path.isfile(candidate):
            return candidate

    return None


def require(*names):
    """Every named tool, or a message saying which one is missing and how to
    get it. A build that fails here has not written anything yet."""
    tools = {}
    missing = []

    for name in names:
        found = find(name)
        if found:
            tools[name] = found
        else:
            missing.append(name)

    if missing:
        raise SystemExit(
            "toolchain: %s not found.\n"
            "  Install LLVM 18 or newer and put its bin directory on PATH, set\n"
            "  BOLT_LLVM to where it is installed, or put clang and lld in\n"
            "  %s. See BUILDING.md, or docs/setup.md in the SDK.\n"
            "  Looked in: %s"
            % (", ".join(missing), BUNDLED,
               os.pathsep.join([BUNDLED] + directories())))

    return tools


def main():
    """What this machine has, which is the first thing to ask when a build will
    not start."""
    print("host    %s" % platform())
    print("target  %s" % TARGET)
    print()

    for name in ("clang-cl", "clang", "lld-link", "llvm-lib", "llvm-objdump"):
        print("  %-14s %s" % (name, find(name) or "not found"))

    print()
    print("searched: %s first%s, then PATH, then:"
          % (BUNDLED, "" if os.path.isdir(BUNDLED) else " (absent)"))

    for where in directories():
        print("  %s%s" % (where, "" if os.path.isdir(where) else "   (absent)"))

    return 0


if __name__ == "__main__":
    sys.exit(main())
