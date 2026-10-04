# Setting up

Three things, on any of three hosts. Nothing else is installed and nothing is
registered: the SDK is a folder.

| | Version | For |
|---|---|---|
| Python | 3.8 or newer | the build and every tool |
| clang and lld | 18 or newer | compiling and linking |
| QEMU | 8 or newer | running the machine; it brings the UEFI firmware |

BoltOS applications are PE/COFF images for x86-64, made by clang-cl and linked
by lld, on whatever the host is. No platform SDK and no C runtime is involved on
any host.

## Windows

- **Python** from python.org, with "Add python.exe to PATH" ticked.
- **LLVM** from the LLVM releases page, the `LLVM-<version>-win64.exe`
  installer, which puts clang and lld in `C:\Program Files\LLVM\bin`. That
  folder is searched without being on PATH.
- **QEMU** from qemu.weilnetz.de (the `qemu-w64-setup` installer). It carries
  `edk2-x86_64-code.fd`, the firmware the machine starts from. For speed, turn
  on the Windows feature *Windows Hypervisor Platform*; without it QEMU
  emulates the processor and everything is a few times slower.

Visual Studio and the Windows SDK are not needed.

## Debian and Ubuntu

```
sudo apt install python3 clang lld llvm qemu-system-x86 ovmf
```

`ovmf` is the firmware, which Debian packages apart from QEMU. For speed, add
yourself to the `kvm` group.

## macOS

```
brew install python llvm lld qemu
```

Homebrew's `llvm` is not on PATH, and its clang is found where Homebrew put it.
Apple's own clang is not used: it has no lld.

## Where the tools look

clang and lld are looked for, in order:

1. `toolchain/bin` in the SDK — where they go when the SDK carries its own;
2. `PATH`;
3. `BOLT_LLVM/bin`, when the environment variable is set;
4. the places each host's packages put them.

`python tools/toolchain.py` says what was found and where it looked.

QEMU is looked for on `PATH` and then where its installer puts it, and the
firmware beside it or in the host's packages.

## Checking it

```
build.cmd examples\empty            ./build.sh examples/empty
run.cmd examples\empty              ./run.sh examples/empty
```

The first should end `built examples\empty\out\debug\empty.bxf`. The second
should open a window with BoltOS starting in it; once the desktop is up, the
Actions menu has **Empty Application** at the top.
