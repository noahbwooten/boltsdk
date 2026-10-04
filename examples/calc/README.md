# The calculator

BoltOS's own calculator, as an application built with the Bolt SDK. It shows
most of what a real application has:

| | |
|---|---|
| `app.ini` | the project: name, version, sources, libraries, the icon resource, the package |
| `src/main.c` | what the system calls: `__modmain`, `__modhalt`, `__modres`, and the two manifests |
| `src/calc.c` | the calculator: retained controls, the keyboard, and 64-bit arithmetic that reports overflow rather than wrapping |
| `src/calc.h` | what the two share |
| `res/calculator.bmp` | the 32 by 32 icon, attached to the file as `AppIcon_Calculator` |

```
build.cmd examples\calc              ./build.sh examples/calc
run.cmd examples\calc                ./run.sh examples/calc
```

The build writes `out/debug/sdkcalc.bxf`, the application, and
`out/debug/sdkcalc.bxi`, its installer. `run` puts the application at the top
of the Actions menu as **SDK Calculator**; `run.cmd
examples\calc\out\debug\sdkcalc.bxi` puts the installer on the desktop instead.

Things worth reading in it:

- **The window procedure** in `calc.c`: controls made at `MSG_INIT`, painted at
  `MSG_PAINT`, taken down at `MSG_QUIT`, and every other message given to
  `CmnCtl2_Dispatch` before the buttons are asked whether they were clicked.
- **The keyboard**, taken before the controls see it, so Return is `=` rather
  than a press of whichever button had the focus.
- **The version**, said once in `app.ini` and reaching the file through
  `boltapp.h` and `MODVERSION_HEADER`.
