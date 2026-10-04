# How an application runs

What a BoltOS application is, how the system runs it, and what that means for
the code in it. Read this before writing anything more than the empty
example.

## A module

An application is one file, a `.bxf`, which is a 64-bit PE image with an
export table, built by clang and lld. It has no entry point of its own and no C
runtime.
The system maps it into memory itself, binds its imports by name against the
system's modules — `user.dll`, `wndmgr.dll`, `cmnctl2.dll` and the rest — and
then finds the functions it calls in the application's export table, by name.
That is why every function the system calls is `__declspec(dllexport)` and
spelled exactly as given.

Each start of an application is a fresh copy of its image, with its own
variables. Two calculators open at once share nothing.

## One line of execution

There are no threads. The system runs a frame loop, many times a second, and
in each frame it calls every running module in turn: each module's clock, then
the window manager, which sends each window the messages that are due and
paints the ones that changed, then the shell. Everything an application does
happens inside one of those calls, and nothing else runs until it returns.

So an application **must return promptly, every time.** It never waits for
something another application or the user will do, because nothing else can
happen while it waits. A long piece of work is cut into pieces and done a
piece a frame, from a timer ([`Wndmgr_SetTimer`](../api/wndmgr.md#wndmgr_settimer)).
The waits the API does offer — on a mutex, on an event — are capped at two
seconds for this reason, and are rarely the right tool.

The same is why the locking calls exist at all: not to stop two lines of
execution colliding, which cannot happen, but to say across frames that
something is in use — a single instance, a file being edited.

## The exports

| | | |
|---|---|---|
| `__modmain` | required | once, when the application starts |
| `__modres` | required | when the system asks what the module carries |
| `__WindowProcedure2` | required for a window | everything that happens to its windows |
| `__modhalt` | optional | once, as the application ends |
| `__modclock` | optional | on the system's clock |
| `__modopen` | optional | a document the application was asked to open |

### __modmain

```c
__declspec(dllexport) int __modmain(void* Reserved);
```

Called once, after the image is mapped and its imports bound. An application
opens its windows here with [`Wndmgr_CreateWindow`](../api/wndmgr.md#wndmgr_createwindow).
Returning does not end it: an application runs for as long as it has a window
open, and the return value is not used.

An application that opens no window is still running when `__modmain`
returns, called on its clock, until something ends it. That is how a service
or a background helper is written; most applications open a window.

### __modres

```c
__declspec(dllexport) WORDPTR __modres(unsigned long Index, unsigned long Query);
```

What the module says about itself, one resource at a time: its name and icon
(`RSRCTYPE_MANIFEST`), its version manifest (`RSRCTYPE_VERSION`), any service
it provides, any System Control page. The system asks how many there are, then
each one's type and data. See [modres](../api/modres.md#what-a-module-describes);
the examples show the usual two.

### __modhalt

```c
__declspec(dllexport) void __modhalt(void);
```

Called once as the application ends — its last window closed, or something
ended it gracefully. Save here; free what is worth freeing. Whatever the
application still holds afterwards is freed for it.

### __modclock

```c
__declspec(dllexport) void __modclock(WORD32 Message);
```

Called on the system's clock with a `MODMSG_*` value: `MODMSG_PAINT` every
frame (a tick, not a chance to draw), `MODMSG_1SEC` once a second, and
`MODMSG_FBRSZ` when the screen changes size. Sent whether or not a window is
showing. A window that wants a regular tick is better served by a timer.

### __modopen

```c
__declspec(dllexport) void __modopen(PSTR Path);
```

Called after `__modmain` when the application was started to open a document
— by the file explorer, or by [`Shell_OpenFileWith`](../api/shellapi.md#shell_openfilewith).
Copy the path; it is not the application's to keep.

## Windows

A window is made with [`Wndmgr_CreateWindow`](../api/wndmgr.md#wndmgr_createwindow),
which looks in the application's exports for its procedure.

### The message-driven procedure

```c
__declspec(dllexport) WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message,
                                                WORD64 Param1, WORD64 Param2);
```

The window is told what has happened — made, painted, clicked, typed at,
resized, closed — one message at a time, and a window with nothing new
costs nothing. This is what a new application writes. The messages are in
[Messages](../api/messages.md). The shape of nearly every procedure:

```c
switch (Message) {
case MSG_INIT:      /* make the controls */                return 0;
case MSG_PAINT:     CmnCtl2_Paint(Window);                 return 0;
case MSG_QUIT:      CmnCtl2_DestroyControls(Window);       return 0;
}

WORD64 Used = CmnCtl2_Dispatch(Window, Message, Param1, Param2);
/* then ask each control CmnCtl2_WasClicked */
return Used;
```

**Drawing happens only in `MSG_PAINT`.** During it the drawing calls point at
the window's own surface, in window coordinates: (0, 0) is the window's
corner and the title bar takes the first `WNDMGR_TITLEH` (30) rows. To change
what is shown, change the state and ask for a paint — controls do that
themselves; anything drawn by hand calls
[`Wndmgr_InvalidateWindow`](../api/wndmgr.md#wndmgr_invalidatewindow).

A window's size is in its handle's slots (`WNDMGRHND_TYPE_WINDOWW` and `_H`),
read with [`User_GetDataForHandle`](../api/user.md#user_getdataforhandle).

### The original procedure

```c
__declspec(dllexport) void __WindowProcedure(WORD32 X, WORD32 Y, WORD32 W,
    WORD32 H, WORD32 MouseX, WORD32 MouseY, BOOL LeftButton, WORD64 Reserved);
```

Called every frame, in screen coordinates, to draw everything from scratch.
Applications written before the message-driven one use it with the
[immediate-mode controls](../api/cmnctl.md). It still works, and is frozen;
an application exporting both is driven by `__WindowProcedure2`.

## Frames

The system keeps track of who is running and how far they are trusted. An
application's own entry points and clock run as the application. A window
procedure is called by the window manager and runs on its behalf — it can
reach every handle, for instance, not only the application's own. Write code
as though it always ran as the application: what a procedure can reach on the
window manager's behalf is not part of the API, and a later system may narrow
it.

As the application, it:

- reaches only the handles it made; another's read as invalid;
- frees only the memory it allocated;
- reads the registry, and writes only `REGHIVE_LOCALCONFIGURATION`;
- may not write, delete or rename the system's files (marked system);
- has no access to hardware and none to the system's own control calls.

## Memory and ownership

Every block from [`User_Allocate`](../api/user.md#user_allocate), every
handle, every window and every control is recorded against the application
that was running when it was made. When the application ends, all of it is
freed, so a leak lasts as long as the application and no longer. Within it,
free what you finish with: Task Manager shows what each application holds.

Something handed to the system by pointer — a window's title, its icon, a
combo box's items, a table row's icon — is *kept*, not copied, unless the
call says otherwise, and must last as long as what it was given to.

## Limits

| | |
|---|---|
| running modules, the system's included | 16 |
| windows on the machine | 64 |
| timers on the machine; on one window | 64; 16 |
| named mutexes and events together | 64 |
| handles | 16384 |
| path length | 1023 characters |

## When something goes wrong

A fault in an application — a bad pointer, a divide by zero — stops the
machine with a dialog naming the module, the instruction's address and the
kind of fault. Dismissing it, with the pointer or with Return or Escape,
closes the application and goes back to the desktop; the rest of the machine
carries on. A fault in the system itself cannot be dismissed. There is no
debugger attached to an application in SDK 10; see
[Running and debugging](running.md) for what there is.
