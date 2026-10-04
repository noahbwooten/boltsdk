# toolchain

Where the SDK's own compiler goes, when it carries one.

The build looks in `toolchain/bin` before anything installed on the machine,
so a copy of LLVM put here is the one every build uses, whatever else is on
`PATH`. It needs these, for the host the SDK is used on:

| Windows | Debian, macOS |
|---|---|
| `bin/clang-cl.exe` | `bin/clang-cl` |
| `bin/lld-link.exe` | `bin/lld-link` |

and whatever libraries they load from beside them. LLVM 18 or newer.

With nothing here, an installed LLVM is used instead; see
[docs/setup.md](../docs/setup.md). `python tools/toolchain.py` says which one
the build will use.
