# Bolt SDK 10 documentation

**For development and evaluation purposes only.**
(c) Noah Wooten 2023-2026, All Rights Reserved.

## Guides

Read in this order the first time.

1. [Setting up](setup.md) — Python, clang and lld, QEMU, on Windows, Debian
   and macOS.
2. [Getting started](guide/getting-started.md) — build the calculator, run it,
   and make an application of your own from the empty one.
3. [How an application runs](guide/application-model.md) — modules, frames,
   the entry points, windows and messages, memory, and what an application may
   and may not do.
4. [The project file](guide/project-file.md) — every key in `app.ini`.
5. [Resources and versions](guide/resources-and-versions.md) — icons and
   sounds, the version manifest, `boltapp.h`.
6. [Packaging](guide/packaging.md) — `.bxi` installers, links and registry
   values.
7. [Running and debugging](guide/running.md) — the disk, `run`, slipstreaming,
   the serial log.
8. [Compatibility](guide/compatibility.md) — `BOLTSDK_APIV`, numbered
   functions, and what never changes.

## Reference

- [API reference](api/index.md) — every function, by header and by name.
- [Messages](api/messages.md) — the exports an application provides and the
  window messages it is sent.
- [Types, errors and folders](api/types.md)
- [The C library subset](api/crt.md)

## The example applications

- `examples/calc` — the calculator: retained controls, the keyboard, an icon
  resource, a version manifest, and a package.
- `examples/empty` — the least an application needs: one window, one line of
  text, and every required export, commented.
