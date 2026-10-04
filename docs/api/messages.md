# Messages

What the system calls in an application, and what it tells it. These are not
functions an application calls, so none of it is numbered: they are the
application's own exports and the values passed to them, and they are frozen
the same way. A message never changes its meaning, and a new one takes a new
number.

## The application's exports

| Export | | Called |
|---|---|---|
| `int __modmain(void* Reserved)` | required | once, when the application starts |
| `WORDPTR __modres(unsigned long Index, unsigned long Query)` | required | whenever the system asks what the module carries; see [modres](modres.md#what-a-module-describes) |
| `WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message, WORD64 Param1, WORD64 Param2)` | for a window | for everything that happens to one of its windows |
| `void __modhalt(void)` | optional | once, as the application ends |
| `void __modclock(WORD32 Message)` | optional | on the system's clock, below |
| `void __modopen(PSTR Path)` | optional | after `__modmain`, with a document to open |
| `void __WindowProcedure(...)` | the original contract | every frame, for a window; see below |

Each is declared `__declspec(dllexport)` and spelled exactly so: the system
finds them by name.

## __modclock

```c
#define MODMSG_PAINT   0x00   /* every frame */
#define MODMSG_1SEC    0x01   /* once a second */
#define MODMSG_WIDGETS 0x02   /* every frame, after the windows */
#define MODMSG_FBRSZ   0x06   /* the screen changed size */
#define MODMSG_USRDEF1 0x03   /* reserved */
#define MODMSG_USRDEF2 0x04   /* reserved */
#define MODMSG_USRDEF3 0x05   /* reserved */
```

Sent to every running module, whether or not it has a window showing.
`MODMSG_PAINT` is a frame tick and not a chance to draw: a message-driven
window draws only in `MSG_PAINT`. A window that wants a tick of its own asks
for one with [`Wndmgr_SetTimer`](wndmgr.md#wndmgr_settimer).

---

## Window messages

`wndproc2.h`. Sent to `__WindowProcedure2`, one at a time, from the window
manager. Numbering starts at one; zero is not a message. Return zero unless a
message says otherwise.

### MSG_INIT

`0x01`. The window has just been made. Sent once, before the first paint.
Build the window's controls here, and allocate what the window needs.

### MSG_KILL

`0x02`. Reserved. When it is sent, the application will be ended 250 ms later
whatever it does.

### MSG_QUIT

`0x03`. The close button was pressed. Return zero to let the window close,
non-zero to refuse — which an application with unsaved work does while it
asks. A refusal should last only as long as the question. Take the window's
controls down here
([`CmnCtl2_DestroyControls`](cmnctl2.md#cmnctl2_destroycontrols)).

### MSG_MINIMIZE

`0x04`. Minimised. Stop drawing and give back anything expensive; the window is
not being destroyed and its state must survive.

### MSG_RESTORE

`0x05`. Showing again after `MSG_MINIMIZE`. Always followed by a paint.

### MSG_PAINT

`0x06`. Draw. The only message during which drawing is valid: the primitives
point at the window's surface for its duration. Drawing is in window
coordinates — (0, 0) is the window's corner, the title bar takes the first
`WNDMGR_TITLEH` rows. `Param1` is the damaged rectangle, packed as
`MSG_MAKERECT`; in SDK 10 it is always the whole window.

### MSG_CURSOR

`0x07`. The pointer moved. `Param1` is where, window-relative, packed as
`MSG_MAKEPOINT`; `Param2` the buttons held, as `1 << code`.

### MSG_KEYDOWN, MSG_KEYUP

`0x08`, `0x09`. A key or mouse button went down or came up; `Param1` is the
key code (see [Keyboard and pointer](user.md#keyboard-and-pointer); the
buttons are `MSGKEY_*`). One message per
change, except that a key held for a second repeats, each repeat another
`MSG_KEYDOWN` with `Param2` set to one.

### MSG_CHAR

`0x0A`. A character was typed; `Param1` is the character. Separate from
`MSG_KEYDOWN`, since shift and the repeat sit between a key and a character.

### MSG_FOCUS, MSG_BLUR

`0x0B`, `0x0C`. The window gained or lost the focus.

### MSG_TIMER

`0x0D`. A second has passed and the window is showing. `Param1` is the time, as
[`User_GetKTime`](user.md#user_getktime) answers it.

### MSG_MOVE

`0x0E`. Reserved.

### MSG_RESIZE

`0x0F`. About to be resized to the size in `Param1`, packed as
`MSG_MAKEPOINT`. Zero accepts, non-zero refuses, so a window that ignores it
accepts. The size has already been held to the window's limits, so lay out
before returning.

### MSG_SCROLL

`0x10`. The wheel turned while the window had the focus. `Param1` is the notch
count, positive away from the user; `Param2` the pointer, packed as
`MSG_MAKEPOINT`.

### MSG_INTERVAL

`0x11`. A timer from [`Wndmgr_SetTimer`](wndmgr.md#wndmgr_settimer) came due.
`Param1` is its identifier, `Param2` the millisecond clock. Sent outside a
paint: change state and invalidate, do not draw.

## Buttons

```c
#define MSGKEY_LBUTTON  0x01
#define MSGKEY_RBUTTON  0x02
#define MSGKEY_MBUTTON  0x04
#define MSGKEY_XBUTTON1 0x05
#define MSGKEY_XBUTTON2 0x06
```

## Packing

```c
MSG_MAKEPOINT(x, y)        MSG_POINTX(p)   MSG_POINTY(p)
MSG_MAKERECT(x, y, w, h)   MSG_RECTX(r)    MSG_RECTY(r)
                           MSG_RECTW(r)    MSG_RECTH(r)
```

Each field is sixteen bits.

---

## The original window contract

```c
__declspec(dllexport) void __WindowProcedure(WORD32 X, WORD32 Y, WORD32 W,
    WORD32 H, WORD32 MouseX, WORD32 MouseY, BOOL LeftButton, WORD64 Reserved);
```

Called every frame for each of the application's windows, in screen
coordinates, with the window's rectangle and the pointer, and expected to draw
everything from scratch. The frame and the client background are drawn
first. This is what the [immediate-mode controls](cmnctl.md) are for. A module
exporting both procedures is driven by `__WindowProcedure2`.

---

## System Control pages

`scitem2.h`. An application can add a page to System Control by describing a
`MODRES_RSRCTYPE_SCITEM2` resource whose procedure has the window procedure's
shape, with a panel where the window would be:

```c
WORD64 __ScItemProcedure2(HANDLE Panel, WORD32 Message, WORD64 Param1,
                          WORD64 Param2);
```

Put the page's controls on the panel at `MSG_INIT`, and hand everything else
to [`CmnCtl2_Dispatch`](cmnctl2.md#cmnctl2_dispatch). A page lives in a module
that may never have been started — System Control finds pages by loading every
application without calling `__modmain` — so it allocates its own state at
`MSG_INIT` and reads nothing its application set up. It also has three
messages of its own:

| | | |
|---|---|---|
| `SCMSG_SHOW` | 0x40 | this page is now the one on screen |
| `SCMSG_HIDE` | 0x41 | another page was chosen; keep everything |
| `SCMSG_RESIZE` | 0x42 | the panel is the size in `Param1`, packed as `MSG_MAKEPOINT` |
| `SCMSG_MINSIZE` | 0x43 | answer the smallest size the page can lay out in, packed the same way; zero for no opinion |
