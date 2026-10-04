# Running and debugging

The SDK carries a BoltOS machine: the loader `image/bootx64.efi` and the
volume `image/fs.bin`, which is BoltOS 2.0 Build 415.0.sdkv10. `run.cmd` and
`run.sh` start it under QEMU, in a window, with your applications on its disk.

## run

```
run.cmd                                 the machine, as its disk was left
run.cmd examples\calc                   a project's newest build put on first
run.cmd C:\work\hello examples\calc     any number of them
run.cmd out\debug\hello.bxf             an application by itself
run.cmd out\debug\hello.bxi             a package, on the desktop
run.cmd --clean examples\calc           a fresh disk first
run.cmd --memory 1024                   anything else goes to run_qemu.py
```

An argument naming a file or a folder is something to put on the disk;
anything else is passed to the machine runner, `tools/run_qemu.py`. Each thing
put on replaces what was there under the same name, so `build` then `run` is
the whole loop.

| What | Goes to |
|---|---|
| a project folder | its newest build, as an application, named and drawn as app.ini says |
| `name.bxf` | `/programs/name/name.bxf`, with a link at the top of the Actions menu |
| `name.bxi` | `/users/default/desktop/name.bxi`, to be installed by opening it |

## The disk

The disk is `run/bolt-disk.img`, made from `image/fs.bin` the first time
`run` is used. It keeps everything the machine writes to it — settings, files,
what was installed — from one run to the next. `--clean` makes it again from
the image, which is the way back to a machine that has never seen your
application. Nothing in `run/` is needed; delete the folder whenever you like.

`tools/slipstream.py` is what puts things on it, and it also takes a file to
put anywhere, as `host=/path`:

```
python tools/slipstream.py --disk run/bolt-disk.img --image image/fs.bin ^
    levels.dat=/programs/hello/levels.dat
```

It refuses to touch a disk a running machine has open; close the machine
first.

## The machine

The machine is a q35 PC with 512 MB, the standard VGA at 1024 by 768, the
disk on AHCI, an RTL8139 network adapter, and AC'97 sound. Useful options for
`run`:

| | |
|---|---|
| `--memory 1024` | more memory |
| `--disk-bus nvme` | the disk on NVMe instead |
| `--no-net`, `--no-audio` | without the adapter or the sound card |
| `--clock 2026-12-31T23:59:00` | start the clock at a given UTC time |

The network is QEMU's user mode: the machine can reach the host as `10.0.2.2`
and the outside world through it.

QEMU's own keys work in its window: Ctrl+Alt+G releases the pointer, and
Ctrl+Alt+F toggles full screen.

### Without a window

`tools/run_qemu.py` runs the machine headless, for a test or a screenshot:

```
python tools/run_qemu.py --efi image/bootx64.efi --fs-image image/fs.bin ^
    --run-dir run --keep-disk --seconds 40 ^
    --actions "click 40 747; wait 1; shot run/menu.png"
```

It runs for `--seconds`, writes `run/screen.png` at the end, and can drive the
pointer and keyboard on the way: `click X Y`, `type text`, `key ret`,
`wait N`, `shot file.png`. `python tools/run_qemu.py --help` has all of it.

## The serial log

Everything the system says about itself goes to `run/serial.log`: every module
loaded and where, every import that could not be bound, faults. When an
application will not start, look here first:

```
[boltos] loader: 'hello.bxf' at 0x000000001a1d6000
```

## Debugging

There is no debugger attached to an application in SDK 10. What there is:

- **Look at it.** Draw what you want to know in the window, or put it in a
  label. A paint costs nothing.
- **Ask.** [`Shell_MessageBox`](../api/shellapi.md#message-boxes) shows a
  value and waits for nobody.
- **Write it down.** Append lines to a file of your own, such as
  `/programs/hello/log.txt`, with
  [`User_OpenFile`](../api/user.md#user_openfile) and `User_WriteFile`, and
  read it in the file explorer or Notepad.
- **`User_GetLastError`** after any call that failed says why.

### A fault

A fault in an application shows a dialog with the module, the kind of fault
and the address of the instruction; dismissing it closes the application. To
turn the address into a function, take the address the serial log says the
module was loaded at, subtract it from the fault's address, add
`0x180000000`, and find the result in `out\debug\<module>.map`, which lists
every function at that base.

## On other machines

`image/` also holds a bootable ISO of the same system. It boots on anything
that starts from UEFI — another emulator, or a real PC from a USB stick —
with the volume held in memory, so what is written there is gone at power off.
`run.cmd` does not use it.
