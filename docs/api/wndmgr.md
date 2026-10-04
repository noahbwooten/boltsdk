# wndmgr.h — windows

`#include <wndmgr.h>` · exported by **wndmgr.dll** · link **`wndmgr.lib`**

The window manager owns every window on the screen: where it is, which one
has the focus, the title bar and its buttons, dragging and resizing, and the
backing surface each window is painted into. An application makes windows
here and is told what happens to them through its window procedure
([messages](messages.md)).

An application stays running for as long as it has a window. Closing its last
window ends it: `__modhalt` is called, and everything it still holds is
freed.

## A window's handle

A window is a `HANDLE`, and its state is in the handle's slots, read with
[`User_GetDataForHandle`](user.md#user_getdataforhandle):

| Slot | Holds |
|---|---|
| `WNDMGRHND_TYPE_WINDOWX`, `_WINDOWY` | its position on the screen |
| `WNDMGRHND_TYPE_WINDOWW`, `_WINDOWH` | its size, title bar included |
| `WNDMGRHND_TYPE_TEXT` | its title, as a `PSTR` |
| `WNDMGRHND_TYPE_ICON` | its icon, as pixels |
| `WNDMGRHND_TYPE_FLAGS` | its `WNDMGRHNDFLAGS_*` |
| `WNDMGRHND_TYPE_MDID` | the module it belongs to |

The title bar is `WNDMGR_TITLEH` (30) pixels tall, and the client area starts
below it. A window is never smaller than 160 by 90.

## Flags

Passed to [`Wndmgr_CreateWindow`](#wndmgr_createwindow); the window manager
sets the rest itself.

| Flag | Effect |
|---|---|
| `WNDMGRHNDFLAGS_NOTASKBARITEM` | no entry on the taskbar |
| `WNDMGRHNDFLAGS_NOMINIMIZE` | no minimise button |
| `WNDMGRHNDFLAGS_NOCLOSE` | no close button |
| `WNDMGRHNDFLAGS_NORESIZE` | no edge to resize it by |
| `WNDMGRHNDFLAGS_CENTERTEXT` | the title centred in the bar |
| `WNDMGRHNDFLAGS_DARKBKGRND` | the darker client background |
| `WNDMGRHNDFLAGS_TOPMOST` | drawn over every window but the focused one |
| `WNDMGRHNDFLAGS_NOICON` | no icon in the title bar (set when the icon is `NULL`) |
| `WNDMGRHNDFLAGS_NOAUTOPAINT` | the window manager does not paint it on its own |

---

## Making and closing windows

### Wndmgr_CreateWindow

```c
HANDLE Wndmgr_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H,
                           PSTR Title, PVOID Icon, WORD32 Flags);
```

wndmgr.dll · export `Wndmgr_CreateWindow1` · SDK 10

Opens a window belonging to the running application and gives it the focus.
Called from `__modmain`, once for each window wanted.

The window is driven by whichever procedure the application exports:
`__WindowProcedure2` if it has one, which is what a new application writes,
or the original `__WindowProcedure`. The first message it is sent is
`MSG_INIT`, before it is first painted.

- `X`, `Y`, `W`, `H` — position and size on the screen, the title bar
  included. Sizes below 160 by 90 are raised to them.
- `Title` — kept, not copied: it must last as long as the window. A string
  literal, or the manifest's name.
- `Icon` — 24-bit pixels, 24 by 24, as [`Wndmgr_LoadIcon`](#wndmgr_loadicon)
  makes them; kept, not copied. `NULL` for none.
- `Flags` — `WNDMGRHNDFLAGS_*`, or zero.
- **Returns** the window, or **0** — not `HANDLE_INVALID` — when the handle
  table is full.

### Wndmgr_CloseWindow

```c
void Wndmgr_CloseWindow(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_CloseWindow1` · SDK 10

Closes a window, without asking it: `MSG_QUIT` is what the close button
sends, and this is not the close button. An application closing its own
window from code takes its controls down first
([`CmnCtl2_DestroyControls`](cmnctl2.md#cmnctl2_destroycontrols)).

A close from inside a window procedure, or from the application whose window
it is, happens after the current message returns, since the window and
possibly the whole application are going away under it. If this was the
application's last window, the application ends.

### Wndmgr_MinimizeWindow

```c
void Wndmgr_MinimizeWindow(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_MinimizeWindow1` · SDK 10

Hides a window to its taskbar entry and gives the focus to one still showing.

### Wndmgr_BringForth

```c
VOID Wndmgr_BringForth(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_BringForth1` · SDK 10

Gives a window the focus and the keyboard, which also puts it on top.

### Wndmgr_FocusVisible

```c
void Wndmgr_FocusVisible(void);
```

wndmgr.dll · export `Wndmgr_FocusVisible1` · SDK 10

Moves the focus to a window that is showing. Making a window focuses it, so
this is for after making one that is not meant to be looked at.

### Wndmgr_ModuleWindowCount

```c
WORD32 Wndmgr_ModuleWindowCount(WORD32 ModuleId);
```

wndmgr.dll · export `Wndmgr_ModuleWindowCount1` · SDK 10

How many windows a module has open. `ModuleId` is a process identifier, as
[`Ps_GetCurrentProcess`](psapi.md#ps_getcurrentprocess) answers.

### Wndmgr_ModuleHasWindows

```c
BOOL Wndmgr_ModuleHasWindows(WORD32 ModuleId);
```

wndmgr.dll · export `Wndmgr_ModuleHasWindows1` · SDK 10

Whether a module has any window open.

### Wndmgr_CloseModuleWindows

```c
void Wndmgr_CloseModuleWindows(WORD32 ModuleId);
```

wndmgr.dll · export `Wndmgr_CloseModuleWindows1` · SDK 10

Closes every window a module holds and leaves the module loaded. What
something ending an application calls first, so no window outlives the code
behind it. An application can reach only its own windows this way.

---

## Painting

### Wndmgr_InvalidateWindow

```c
void Wndmgr_InvalidateWindow(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_InvalidateWindow1` · SDK 10

Marks a window as needing repainting, so it is sent `MSG_PAINT`. A window
using the retained controls never needs this: every setter marks the window
itself. An application drawing something of its own calls it when that
changes — from a timer, say — and draws in `MSG_PAINT`.

### Wndmgr_GetColors

```c
PWNDMGR_COLORS Wndmgr_GetColors(void);
```

wndmgr.dll · export `Wndmgr_GetColors1` · SDK 10

The palette the system draws with, as a `WNDMGR_COLORS` the window manager
owns: text, backgrounds, the accent, the taskbar's, the windows', the
controls'. Read it; do not write it.

```c
Ge_DrawText(16, WNDMGR_TITLEH + 16, "Hello",
            Wndmgr_GetColors()->TextColor, GEFONT_NORMAL);
```

### Wndmgr_LoadIcon

```c
PVOID Wndmgr_LoadIcon(PSTR ResourceName, WORD32 Size);
```

wndmgr.dll · export `Wndmgr_LoadIcon1` · SDK 10

A picture the calling application carries, scaled to `Size` by `Size`, as
24-bit pixels in a block the caller owns. A name the application does not
carry is looked for in the shell. Square sizes only; the title bar draws 24,
the Actions menu 32. **Returns** `NULL` for a name nobody carries or an empty
one.

### Wndmgr_LoadIconFrom

```c
PVOID Wndmgr_LoadIconFrom(PSTR Opt_ModulePath, PSTR ResourceName, WORD32 Size);
```

wndmgr.dll · export `Wndmgr_LoadIconFrom1` · SDK 10

The same, out of the module a path names, which is how another application's
icon is drawn without starting it. `NULL` is the calling application, as
above.

### Wndmgr_SetWindowCursor

```c
void Wndmgr_SetWindowCursor(HANDLE Window, WORD32 Cursor);
```

wndmgr.dll · export `Wndmgr_SetWindowCursor1` · SDK 10

What the pointer looks like over the window's client area, from then on:
`CURSOR_POINT`, `CURSOR_ACTION` (the hand), `CURSOR_TEXT` (the I-beam),
`CURSOR_WAIT`, or one of the resize shapes (`cursor.h`). Set it when it
changes, not every frame.

### Wndmgr_DrawWindow

```c
void Wndmgr_DrawWindow(HANDLE Window, BOOL WindowHasFocus);
```

wndmgr.dll · export `Wndmgr_DrawWindow1` · SDK 10

Draws a window's frame and title bar onto the screen as the window manager
does each frame. The window manager calls it; an application has no reason to.

---

## Scrolling a window

A window whose contents are taller than it is can say so, and the window
manager gives it a scroll bar, scrolls it with the wheel, and hands it paint
and pointer messages in content coordinates. The application draws its whole
layout at every size and lets the window manager decide what is on screen.

### Wndmgr_SetContentHeight

```c
VOID Wndmgr_SetContentHeight(HANDLE Window, WORD32 Height);
```

wndmgr.dll · export `Wndmgr_SetContentHeight1` · SDK 10

How tall the client area's contents are. Taller than the window, and the
window grows a bar down its right edge. Zero turns scrolling off.

### Wndmgr_GetScrollY

```c
WORD32 Wndmgr_GetScrollY(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_GetScrollY1` · SDK 10

How far down its contents the window is looking, in pixels.

### Wndmgr_VisibleWidth

```c
WORD32 Wndmgr_VisibleWidth(HANDLE Window);
```

wndmgr.dll · export `Wndmgr_VisibleWidth1` · SDK 10

How wide the client area can be seen: the whole width, or up to the scroll
bar while there is one. For laying out so nothing sits under the bar.

---

## Scroll bars by hand

The geometry and the painter every scroll bar in the system uses, shared so a
bar an application draws behaves like the others. Positions are in items, or
pixels — whatever `Total` and `Visible` count. A bar is `WNDMGR_SCROLL_W` (14)
pixels wide.

```c
#define WNDMGR_SCROLLPART_NONE     0
#define WNDMGR_SCROLLPART_UP       1
#define WNDMGR_SCROLLPART_PAGEUP   2
#define WNDMGR_SCROLLPART_THUMB    3
#define WNDMGR_SCROLLPART_PAGEDOWN 4
#define WNDMGR_SCROLLPART_DOWN     5
```

[`CmnCtl2_CreateScrollBar`](cmnctl2.md#cmnctl2_createscrollbar) does all of
this for you.

### Wndmgr_ScrollNeeded

```c
BOOL Wndmgr_ScrollNeeded(WORD32 Total, WORD32 Visible);
```

wndmgr.dll · export `Wndmgr_ScrollNeeded1` · SDK 10

Whether there is more than fits: `Total` greater than `Visible`.

### Wndmgr_ScrollMax

```c
WORD32 Wndmgr_ScrollMax(WORD32 Total, WORD32 Visible);
```

wndmgr.dll · export `Wndmgr_ScrollMax1` · SDK 10

The furthest position: the one with the last item at the bottom. Zero when
everything fits.

### Wndmgr_ScrollClamp

```c
WORD32 Wndmgr_ScrollClamp(WORD32 Total, WORD32 Visible, int Position);
```

wndmgr.dll · export `Wndmgr_ScrollClamp1` · SDK 10

A position brought back between zero and the furthest.

### Wndmgr_PaintScrollBar

```c
VOID Wndmgr_PaintScrollBar(WORD32 X, WORD32 Y, WORD32 H, WORD32 Total,
                           WORD32 Visible, WORD32 Position, BOOL Disabled);
```

wndmgr.dll · export `Wndmgr_PaintScrollBar1` · SDK 10

Draws a bar `H` tall at (`X`, `Y`): an arrow at each end and the thumb sized
and placed for the position. Disabled, or with nothing to scroll, it is drawn
flat with no thumb.

### Wndmgr_ScrollPart

```c
WORD32 Wndmgr_ScrollPart(WORD32 Y, WORD32 H, WORD32 Total, WORD32 Visible,
                         WORD32 Position, WORD32 PointY);
```

wndmgr.dll · export `Wndmgr_ScrollPart1` · SDK 10

Which part of a bar a point at height `PointY` is on: a
`WNDMGR_SCROLLPART_*`.

### Wndmgr_ScrollStep

```c
WORD32 Wndmgr_ScrollStep(WORD32 Part, WORD32 Total, WORD32 Visible,
                         WORD32 Position);
```

wndmgr.dll · export `Wndmgr_ScrollStep1` · SDK 10

The position after a click on a part: one item for an arrow, a page for the
track, clamped. The thumb leaves it where it is.

### Wndmgr_ScrollGrab

```c
WORD32 Wndmgr_ScrollGrab(WORD32 Y, WORD32 H, WORD32 Total, WORD32 Visible,
                         WORD32 Position, WORD32 PointY);
```

wndmgr.dll · export `Wndmgr_ScrollGrab1` · SDK 10

Where on the thumb a press at `PointY` landed, to keep for the drag that
follows.

### Wndmgr_ScrollFromPoint

```c
WORD32 Wndmgr_ScrollFromPoint(WORD32 Y, WORD32 H, WORD32 Total,
                              WORD32 Visible, WORD32 Grab, WORD32 PointY);
```

wndmgr.dll · export `Wndmgr_ScrollFromPoint1` · SDK 10

The position for the thumb dragged to `PointY`, with `Grab` from
[`Wndmgr_ScrollGrab`](#wndmgr_scrollgrab).

---

## Timers

### Wndmgr_SetTimer

```c
WORD32 Wndmgr_SetTimer(HANDLE Window, WORD32 IntervalMs);
```

wndmgr.dll · export `Wndmgr_SetTimer1` · SDK 10

Asks for `MSG_INTERVAL` every `IntervalMs` milliseconds, the first one
interval from now. Checked once a frame, so it is a deadline rather than a
promise. A window may hold `WNDMGR_MAX_TIMERS` (16), and the system 64.

- **Returns** an identifier, carried in `Param1` of each `MSG_INTERVAL`, or 0
  when there is no room or the interval is zero. Timers go with the window.

### Wndmgr_KillTimer

```c
void Wndmgr_KillTimer(HANDLE Window, WORD32 Id);
```

wndmgr.dll · export `Wndmgr_KillTimer1` · SDK 10

Stops a timer before its window closes. The window is named as well, so one
window cannot stop another's by guessing.

---

## The desktop and the shell

These serve the shell and the controls, and are documented for completeness.
An application has little reason to call them.

### Wndmgr_SetDesktopPainter

```c
VOID Wndmgr_SetDesktopPainter(PVOID Painter);
```

wndmgr.dll · export `Wndmgr_SetDesktopPainter1` · SDK 10

Registers what paints the background, called once a frame between the
backdrop and the first window as `void Painter(WORD32 Width, WORD32 Height)`.
There is one desktop and one painter, and the shell has it.

### Wndmgr_WindowAtPoint

```c
HANDLE Wndmgr_WindowAtPoint(WORD32 X, WORD32 Y);
```

wndmgr.dll · export `Wndmgr_WindowAtPoint1` · SDK 10

The window drawn on top at a point on the screen, or `HANDLE_INVALID`.

### Wndmgr_ListWindowAtPoint

```c
HANDLE Wndmgr_ListWindowAtPoint(WORD32 X, WORD32 Y);
```

wndmgr.dll · export `Wndmgr_ListWindowAtPoint1` · SDK 10

The window whose open drop-down list a point is on, where the list hangs past
the window's edge, or `HANDLE_INVALID`.

### Wndmgr_SetOverlay

```c
VOID Wndmgr_SetOverlay(WORD32 Count, const WORD16* Rects);
```

wndmgr.dll · export `Wndmgr_SetOverlay1` · SDK 10

What the shell draws over the windows — the taskbar, the Actions menu — as up
to `WNDMGR_OVERLAY_MAX` (8) rectangles of four `WORD16`s each. A press on one
is the shell's and no window's. The shell sets it every frame.

### Wndmgr_KeysOnDesktop

```c
BOOL Wndmgr_KeysOnDesktop(VOID);
```

wndmgr.dll · export `Wndmgr_KeysOnDesktop1` · SDK 10

Whether the keyboard belongs to the desktop: a press on empty desktop takes it
from the focused window, which stays on top but is sent no keys.

### Wndmgr_GetShellRes

```c
PKMCTL_SHELLRES Wndmgr_GetShellRes(VOID);
```

wndmgr.dll · export `Wndmgr_GetShellRes1` · SDK 10

The shell's resources as the system keeps them — the cursors, the background,
the clip — as a `KMCTL_SHELLRES`. Read, not written.

### Wndmgr_SetClips

```c
VOID Wndmgr_SetClips(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
```

wndmgr.dll · export `Wndmgr_SetClips1` · SDK 10

Sets the rectangle the window manager holds for the window it is drawing, for
the original immediate-mode controls. Drawing is clipped with
[`Ge_SetClip`](graphics.md#ge_setclip).

### Wndmgr_GetWindowData

```c
VOID Wndmgr_GetWindowData(PWORD16 MouseX, PWORD16 MouseY, PBOOL LeftButton);
```

wndmgr.dll · export `Wndmgr_GetWindowData1` · SDK 10

The pointer as the window being drawn was handed it, for the immediate-mode
controls. `MouseX` and `MouseY` must not be `NULL`; `LeftButton` may be.

### Wndmgr_RequestSpecialObject

```c
PVOID Wndmgr_RequestSpecialObject(VOID);
```

wndmgr.dll · export `Wndmgr_RequestSpecialObject1` · SDK 10

A pointer to the window manager's own state, for the controls library. Its
layout is not part of the API and changes between builds; an application must
not read it.
