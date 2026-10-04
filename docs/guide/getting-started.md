# Getting started

This builds the calculator, runs it, and then makes an application of your
own. It assumes [Setting up](../setup.md) is done. Commands are given for
Windows; on Debian and macOS, `./build.sh` and `./run.sh` take the same
arguments.

## Build the calculator

From the SDK's folder:

```
build.cmd examples\calc
```

```
SDK Calculator 1.0.0.0, debug, Bolt SDK 10
  toolchain    C:\Program Files\LLVM\bin\clang-cl.exe
  src\main.c
  src\calc.c
  sdkcalc.bxf  19020 bytes
  carries      1 resources, 3636 bytes
  sdkcalc.bxi  7606 bytes

built examples\calc\out\debug\sdkcalc.bxf
```

That compiled the two sources against the SDK's headers, linked them against
`user.lib`, `wndmgr.lib` and `cmnctl2.lib`, checked that the file says it is
version 1.0.0.0 and was built for SDK 10, attached the calculator's icon to
it, and wrote an installer package beside it.

## Run it

```
run.cmd examples\calc
```

This puts the calculator on the machine's disk, in
`/programs/sdkcalc/sdkcalc.bxf`, with a link at the top of the Actions menu,
and starts the machine in a window. When the desktop is up, open **Actions**
and choose **SDK Calculator**. Type `12+30=` and it answers 42.

Close the machine's window to stop it. The disk is kept, so the next `run.cmd`
starts where this one left off; `run.cmd --clean` starts from a fresh one.

## Install it the other way

The build also made `sdkcalc.bxi`, the calculator as an installer package:

```
run.cmd --clean examples\calc\out\debug\sdkcalc.bxi
```

`--clean` starts from a fresh disk, which is the machine somebody else would
have. The package is on the desktop. Double-click it and the installer offers
to install the calculator into `/programs/sdkcalc`, with links in the Actions
menu and on the desktop; **Next**, **Install**, and it is there. That is how
somebody else would install your application.

## Make your own

Copy `examples\empty` to a folder of your own — anywhere; it need not be in
the SDK:

```
xcopy /e /i examples\empty C:\work\hello
```

Open `C:\work\hello\app.ini` and change the `[application]` section. `module`
is the file's name on the machine, so it must be different from every other
application's:

```ini
[application]
name        = Hello
module      = hello
version     = 0.1.0.0
description = Hello
product     = Hello
copyright   = Copyright (c) You 2026
icon        =
```

Build it and run it, naming the folder:

```
build.cmd C:\work\hello
run.cmd C:\work\hello
```

**Hello** is in the Actions menu, and opens a window with a line of text in it.

## What to change next

`src\main.c` is the whole application. It has the five things every
application has — the two manifests, `__modres`, `__modmain`, and a window
procedure — each commented with what it is for.
[How an application runs](application-model.md) explains them.

From there:

- **Controls.** Add buttons, text boxes, tables and trees with the
  [retained controls](../api/cmnctl2.md): make them in `MSG_INIT`, paint them
  in `MSG_PAINT`, and ask `CmnCtl2_WasClicked` after `CmnCtl2_Dispatch`. Add
  `cmnctl2` to `imports` in app.ini. The calculator is a whole example.
- **An icon.** Draw a 32 by 32 bitmap, add it under `[resources]` as
  `AppIcon_Hello`, and set `icon = AppIcon_Hello`. See
  [Resources and versions](resources-and-versions.md).
- **A package.** Add a `[package]` section, and the build writes an installer
  beside the application. See [Packaging](packaging.md).
- **Settings.** Keep preferences in the registry, under
  `REGHIVE_LOCALCONFIGURATION`. See [registry.h](../api/registry.md).
- **Files.** The file calls are in [user.h](../api/user.md#files). Keep an
  application's own files in its folder under `/programs`.

## Editing with an editor

The build writes `out\debug\compile_commands.json`, which tells an editor
exactly how each file was compiled — the defines, `BOLTSDK_APIV`, and where
the headers are. Point clangd, or Visual Studio Code's C/C++ extension, at it
and completion and errors match the build.
