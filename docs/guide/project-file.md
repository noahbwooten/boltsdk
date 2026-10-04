# The project file

A project is a folder with an `app.ini` in it. The file says everything about
an application that is not C, and `build.cmd`, `run.cmd` and the packager read
it. Paths in it are relative to the folder it is in.

It is an INI file: `[sections]`, `key = value` lines, and comments that start
with `;` or `#`. Keys are written as shown; registry paths keep their case.

```ini
[application]
name        = SDK Calculator
module      = sdkcalc
version     = 1.0.0.0
description = Calculator
product     = Bolt SDK Examples
copyright   = Copyright (c) Noah Wooten 2023-2026
icon        = AppIcon_Calculator

[build]
sdk     = 10
sources = src/main.c src/calc.c
imports = user wndmgr cmnctl2

[resources]
AppIcon_Calculator = res/calculator.bmp, no-compress

[package]
appid     = sdkcalc
publisher = Bolt SDK Examples
desktop   = default

[registry]
Software/sdkcalc/InstalledFrom = string:Bolt SDK
```

## [application]

What the application is. Required. Each of these reaches the code as a
`BOLTAPP_*` definition in `boltapp.h`, which the build writes before it
compiles anything; see [Resources and versions](resources-and-versions.md).

| Key | | Default | Becomes |
|---|---|---|---|
| `name` | required | | `BOLTAPP_NAME`: what the application is called, in its window and the Actions menu. At most 63 characters. |
| `module` | required | | `BOLTAPP_MODULE`: the file's name on the machine, `<module>.bxf`, and its folder under `/programs`. Letters, digits, `-` and `_`. Two applications on one machine need two different ones. |
| `version` | | `1.0.0.0` | `BOLTAPP_VERSION_MAJOR`, `_MINOR`, `_BUILD`, `_REVISION`: one to four numbers, each up to 65535. |
| `description` | | the name | `BOLTAPP_DESCRIPTION`: the version manifest's description, which Properties shows. |
| `product` | | the name | `BOLTAPP_PRODUCT`: the product it belongs to. |
| `copyright` | | empty | `BOLTAPP_COPYRIGHT` |
| `language` | | `English` | `BOLTAPP_LANGUAGE` |
| `icon` | | empty | `BOLTAPP_ICON`: the name of a picture under `[resources]` that is the application's icon. Empty for none. |

`BOLTAPP_FILENAME` is `<module>.bxf`, and `BOLTAPP_VERSION_STRING` the version
written out.

## [build]

How it is compiled. `sources` is required.

| Key | Default | |
|---|---|---|
| `sdk` | the latest | The Bolt API version to build against, which becomes `BOLTSDK_APIV`. An application asks for the oldest SDK that has what it needs, so it runs on the most systems. See [Compatibility](compatibility.md). |
| `sources` | | The C files, separated by spaces. |
| `imports` | `user` | The system libraries to link against: `user`, `wndmgr`, `cmnctl`, `cmnctl2`, `psapi`, `netapi`, `boltshell`. Each header's page in the reference says which library it needs. |
| `defines` | | Extra preprocessor definitions, separated by spaces: `DEBUG_LOG FEATURE=2`. |
| `include` | | Extra folders to look for headers in, separated by spaces. |

Every source is compiled with `BOLTSDK_APIV`, `_USERMODE_APP`, and `_DEBUG` or
`NDEBUG`, as C, freestanding: there are no system headers and no C runtime,
only the SDK's `include` folder and the [C library subset](../api/crt.md).
Debug builds are unoptimised and carry DWARF and CodeView; `--release`
optimises (`/O2`, link-time optimisation, unused functions dropped).

## [resources]

The pictures the application carries, attached to its file after the link.
One line each:

```ini
Name = path/to/picture.bmp
Name = path/to/picture.bmp, no-compress, no-flip
```

The name is what the code asks for —
[`Wndmgr_LoadIcon`](../api/wndmgr.md#wndmgr_loadicon),
[`UserRes_LoadImage`](../api/modres.md#userres_loadimage). Pictures are
24-bit (or 1, 4 or 8-bit paletted) uncompressed `.bmp` files; magenta
(255, 0, 255) is transparent. An application icon is 32 by 32 and named
`AppIcon_` something.

| Option | |
|---|---|
| `no-compress` | stored as it is rather than run-length compressed; for a picture that compresses badly |
| `no-flip` | the bitmap's rows are already top-down; the usual bottom-up `.bmp` is flipped |

## [sounds]

The sounds the application carries, the same way:

```ini
Sound_Chime = res/chime.wav
```

A sound is a PCM `.wav`, converted to the mixer's rate as it is attached. The
application registers it once with
[`User_RegisterSound`](../api/modres.md#user_registersound), then plays it by
name with [`User_PlaySound`](../api/user.md#user_playsound).

## [package]

Present, and the build also writes `<module>.bxi`, an installer package; see
[Packaging](packaging.md).

| Key | Default | |
|---|---|---|
| `appid` | the module | The application's identity: its folder under `/programs` and its key in the registry's list of what is installed. At most 31 characters. |
| `publisher` | `Unknown` | Who made it, as the installer shows. |
| `version` | the version | The version the installer shows. |
| `icon` | the application's | The picture the installer and the links are drawn with. |
| `actions` | `yes` | A link in the Actions menu. |
| `desktop` | `optional` | A link on the desktop: `none`; `optional`, offered and unticked; `default`, offered and ticked; `always`, made without asking. |
| `files` | | Other files installed beside the application, as `host=installed` pairs: `data/levels.dat=levels.dat`. |

## [registry]

Values the installer writes and takes away again on uninstall. Only under
`Software/<appid>` in `REGHIVE_LOCALMACHINE`; the installer refuses anything
else.

```ini
Software/sdkcalc/InstalledFrom = string:Bolt SDK
Software/sdkcalc/Level         = dword:3
```

The key path and value name come before the `=`, the last component being the
name; the type and value after it: `string:`, `qword:`, `dword:`, `word:` or
`boolean:`. Numbers may be written in hex as `0x10`.
