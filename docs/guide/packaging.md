# Packaging

A `.bxi` is an application in one file, ready for somebody else to install:
its module and any other files compressed, the registry values it wants, and
the links it should be reachable by. Opening one runs the installer, which is
part of BoltOS, so a package is all anybody needs to be given.

## Making one

Add a `[package]` section to `app.ini` and build. The package is written
beside the application, as `out\debug\<module>.bxi` (or `out\release`), and
read straight back to check every member inflates to what was packed.

```ini
[package]
appid     = sdkcalc
publisher = Bolt SDK Examples
desktop   = default
```

Every key is in [The project file](project-file.md#package). The application's
module always goes in, under its `<module>.bxf` name; anything else it needs is
listed under `files`.

`tools/mkbxi.py --check <package.bxi>` prints what a package holds and checks
it again.

## What installing does

Opening a package — double-clicking it, or
[`Shell_OpenFile`](../api/shellapi.md#shell_openfile) on it — starts the
installer, which shows what the package is and asks:

- **where**, defaulting to `/programs/<appid>`;
- **which optional links** to make — a desktop link under `desktop = optional`
  or `default` is a tick box.

Then it writes, in order: every file into the folder; every registry value;
every link; and a copy of the package itself, as `setup.bxi` in the folder,
which is what removes the application later. What was done is written down
under `Software/Installed/<appid>` in `REGHIVE_LOCALMACHINE`.

Links point at the installed module: an Actions menu link named for the
application, drawn with its icon, and the desktop one if it was kept.

## Removing

System Control's **Installed Applications** page lists everything installed
from a package, with its version, publisher and size. Removing one there runs
the installer on the kept `setup.bxi`, which takes back exactly what the
install did — links, values, files and folder — and the record with them.

## Registry values

A package may write values only under `Software/<appid>` in
`REGHIVE_LOCALMACHINE`, and the installer refuses one that reaches anywhere
else; they are removed with the application. They are the place for something
that describes the installation. Preferences that change as the application is
used belong in `REGHIVE_LOCALCONFIGURATION`, written by the application
itself.

## Trying it

```
run.cmd examples\calc\out\debug\sdkcalc.bxi
```

puts the package on the machine's desktop. Use `run.cmd --clean` first to try
it on a machine that has never seen the application.

## The format

`include/bxi.h` describes a package record by record — a header, a table of
files, a table of registry values, a table of links, then the compressed
members (raw deflate) — for anything that wants to read or write one itself.
`tools/mkbxi.py` is the SDK's writer and reader.
