# Bolt SDK 10

**For development and evaluation purposes only.**
(c) Noah Wooten 2023-2026, All Rights Reserved.

Everything needed to write applications for BoltOS: the headers and libraries
of the Bolt API, the tools that build, package and install an application, a
BoltOS machine to run it on, two example applications, and the documentation.
It targets **BoltOS 2.0 Build 415.0.sdkv10**, the system on the disk in
`image/`, and any later system that provides SDK 10 or newer.

The folder is self-contained. Unzip it anywhere; nothing in it refers to
anything outside it but the tools below.

## What you need

| | |
|---|---|
| **Python** 3.8 or newer | runs the build and every tool |
| **clang and lld** 18 or newer | compile and link; or put them in `toolchain/` |
| **QEMU** | runs the machine, and brings its own UEFI firmware |

[docs/setup.md](docs/setup.md) has the packages for Windows, Debian and macOS.

## In five minutes

```
build.cmd examples\calc          ./build.sh examples/calc
run.cmd examples\calc            ./run.sh examples/calc
```

The first builds the calculator into `examples/calc/out/debug/sdkcalc.bxf`.
The second puts it on the machine's disk and starts the machine in a window:
open **Actions**, and **SDK Calculator** is at the top. Close the window to
stop it.

To start an application of your own, copy `examples/empty` to a folder of
your own, change the `[application]` section of its `app.ini`, and build that.
[Getting started](docs/guide/getting-started.md) walks through it.

## What is here

| | |
|---|---|
| `build.cmd`, `build.sh` | build a project: `build.cmd <folder> [--release] [--clean]` |
| `run.cmd`, `run.sh` | put applications and packages on the disk, and start the machine |
| `include/` | the Bolt API's headers |
| `lib/` | the import libraries, one per system module, and the `.def` each was made from |
| `tools/` | the build, the packager, the disk tools and the machine runner, in Python |
| `toolchain/` | empty: clang and lld go here when the SDK carries its own |
| `image/` | the machine: `bootx64.efi`, the volume `fs.bin`, and a bootable ISO |
| `examples/calc` | a complete application: controls, keyboard, an icon, a version, a package |
| `examples/empty` | the least an application needs to run |
| `docs/` | the guides and the API reference, in Markdown and HTML |

## Documentation

Open [docs/index.html](docs/index.html) in a browser, or read the Markdown
beside it.

- [Setting up](docs/setup.md)
- [Getting started](docs/guide/getting-started.md)
- [How an application runs](docs/guide/application-model.md)
- [The project file](docs/guide/project-file.md)
- [Resources and versions](docs/guide/resources-and-versions.md)
- [Packaging](docs/guide/packaging.md)
- [Running and debugging](docs/guide/running.md)
- [Compatibility](docs/guide/compatibility.md)
- [API reference](docs/api/index.md)

## The promise

An application built against SDK 10 keeps running on every later BoltOS. A
function in the Bolt API never changes its signature and is never removed; a
change is a second, numbered copy beside the first, and an application chooses
which SDK it is built against with `BOLTSDK_APIV`.
[Compatibility](docs/guide/compatibility.md) has the whole of it.
