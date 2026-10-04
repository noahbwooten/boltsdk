# user.h — the core

`#include <user.h>` · exported by **user.dll** · link **`user.lib`**

Everything an application does that is not drawing a control goes through
here: handles, memory, the clocks, the keyboard and the pointer, files and
folders, sound, the system's version, services, the command line, and the
three ways to say that something is in use. Every application includes this
header, and it brings `wndproc2.h` (the window messages) and `modres.h` (what
a module carries) with it.

Each function is listed under the name an application writes. Built against
the SDK, that name calls the numbered export shown beside it, which is the one
that never changes; see [Compatibility](../guide/compatibility.md).

Unless an entry says otherwise, a function that fails records why in the last
error, which [`User_GetLastError`](#user_getlasterror) reads back; the values
are the `ERROR_*` codes in [Types and errors](types.md#errors).

---

## Handles

A `HANDLE` names something the system holds for an application: a window, a
file, a control, a mutex. It is an opaque number, not a pointer, and
`HANDLE_INVALID` is the value every call answers with when it could not make
one. A handle belongs to the module that was running when it was made, and an
application can reach only its own: another application's handle reads as
invalid rather than as refused.

A handle also carries numbered slots, which is where windows keep their
geometry (`WNDMGRHND_TYPE_*` in [wndmgr.h](wndmgr.md)) and where an
application may keep values of its own on a handle it made.

### User_CreateHandle

```c
HANDLE User_CreateHandle(BYTE HandleType, PWORD32 HandleData, int DataCount);
```

user.dll · export `User_CreateHandle1` · SDK 10

Makes a handle of the given type, owned by the calling application, with its
first `DataCount` slots copied from `HandleData`. An application making a
handle for its own bookkeeping uses `HANDLETYPE_GENERIC`; the other types
belong to the calls that make them.

- `HandleType` — a `HANDLETYPE_*` value.
- `HandleData` — the initial slot values, or `NULL` when `DataCount` is 0.
- `DataCount` — how many to copy, at most 32; more are ignored.
- **Returns** the handle, or `HANDLE_INVALID` when the system's handle table
  is full (16384 handles, at which point something is leaking them).

### User_DestroyHandle

```c
VOID User_DestroyHandle(HANDLE Handle);
```

user.dll · export `User_DestroyHandle1` · SDK 10

Gives a handle back. A file handle is closed by this. The value must not be
used afterwards, since the slot it names is handed out again.

Sets `ERROR_INVALID_HANDLE` and does nothing for a handle that is not valid or
not the caller's.

### User_IsHandleValid

```c
BOOL User_IsHandleValid(HANDLE Handle);
```

user.dll · export `User_IsHandleValid1` · SDK 10

Whether a handle is live and reachable by the caller. Another application's
handle answers `FALSE`. A window procedure runs on the window manager's
behalf and can reach every handle; see the
[application model](../guide/application-model.md#frames).

### User_GetDataForHandle

```c
WORDPTR User_GetDataForHandle(HANDLE Handle, WORD32 Position);
```

user.dll · export `User_GetDataForHandle1` · SDK 10

Reads one slot of a handle. A slot that was never written reads as zero.

- `Position` — the slot, below 512. A window's are `WNDMGRHND_TYPE_*`:
  `User_GetDataForHandle(Window, WNDMGRHND_TYPE_WINDOWW)` is its width.
- **Returns** the value, or 0 with `ERROR_INVALID_HANDLE` or
  `ERROR_INVALID_ARGUMENT` set.

### User_SetDataForHandle

```c
VOID User_SetDataForHandle(HANDLE Handle, WORD32 Position, WORDPTR Data);
```

user.dll · export `User_SetDataForHandle1` · SDK 10

Writes one slot of a handle. Slots past the first 32 are made the first time
they are written. Writing a window's geometry slots changes what the window
manager believes about it, which is how an application resizes its own window
from code.

Sets `ERROR_INVALID_HANDLE`, `ERROR_INVALID_ARGUMENT` for a position of 512 or
more, or `ERROR_MEMORY_FAILURE`.

---

## Indivisible operations

One instruction each, none of them a call into the kernel. For a counter that
an interrupt, a timer callback or another application's frame might touch in
the middle of a read, a sum and a write. Add and exchange answer with the
value that was there before; increment and decrement answer with the value
after, since a caller of those is usually asking whether it reached zero. A
`NULL` pointer answers 0 (or `FALSE`) and touches nothing.

### UserAtomic_Read

```c
WORD32 UserAtomic_Read(volatile WORD32* Where);
```

user.dll · export `UserAtomic_Read1` · SDK 10

The value at `Where`, read in one access.

### UserAtomic_Increment

```c
WORD32 UserAtomic_Increment(volatile WORD32* Where);
```

user.dll · export `UserAtomic_Increment1` · SDK 10

Adds one. **Returns** the value after.

### UserAtomic_Decrement

```c
WORD32 UserAtomic_Decrement(volatile WORD32* Where);
```

user.dll · export `UserAtomic_Decrement1` · SDK 10

Takes one away. **Returns** the value after.

### UserAtomic_Add

```c
WORD32 UserAtomic_Add(volatile WORD32* Where, WORD32 Value);
```

user.dll · export `UserAtomic_Add1` · SDK 10

Adds `Value`, wrapping as unsigned arithmetic does. **Returns** the value
before.

### UserAtomic_Exchange

```c
WORD32 UserAtomic_Exchange(volatile WORD32* Where, WORD32 Value);
```

user.dll · export `UserAtomic_Exchange1` · SDK 10

Writes `Value`. **Returns** the value before.

### UserAtomic_CompareExchange

```c
BOOL UserAtomic_CompareExchange(volatile WORD32* Where, PWORD32 Expected,
                                WORD32 Value);
```

user.dll · export `UserAtomic_CompareExchange1` · SDK 10

Writes `Value` only if `*Where` still equals `*Expected`. Either way,
`*Expected` is replaced by what was found, so a loop does not have to read
again at the top.

- **Returns** `TRUE` if it wrote.

---

## Mutexes

Ownership of something across frames, by name, so that two applications reach
the same one without either having had to create it. In a machine with one
line of execution this says that a thing is in use; it does not make a
sequence indivisible, since another application only ever runs between this
one's frames. Single instance, a file open for editing, a long operation under
way.

Mutexes are recursive: the owner taking it again succeeds and must release it
as many times. There are 64 named objects (mutexes and events together) in the
machine.

No wait is longer than `USER_WAIT_LONGEST` (2000 ms), whatever is asked for.
An application waiting for a lock that another application holds is waiting
for something that cannot run until it returns, so the wait is a formality and
the answer is no. Prefer [`User_TryMutex`](#user_trymutex).

### User_CreateMutex

```c
HANDLE User_CreateMutex(PSTR Name);
```

user.dll · export `User_CreateMutex1` · SDK 10

Opens the mutex with this name, creating it if nothing has. Two applications
opening one name get the same mutex.

- **Returns** a handle to it, or `HANDLE_INVALID` with
  `ERROR_OUT_OF_RESOURCES` when every named object is in use.

### User_TryMutex

```c
BOOL User_TryMutex(HANDLE Mutex);
```

user.dll · export `User_TryMutex1` · SDK 10

Takes the mutex if it is free, and never waits. **Returns** `TRUE` if the
caller now holds it. The one to use.

### User_AcquireMutex

```c
BOOL User_AcquireMutex(HANDLE Mutex, WORD32 TimeoutMs);
```

user.dll · export `User_AcquireMutex1` · SDK 10

Takes the mutex, waiting up to `TimeoutMs` (never more than
`USER_WAIT_LONGEST`). **Returns** `FALSE` on expiry, which is an answer and
not a failure of the call.

### User_ReleaseMutex

```c
VOID User_ReleaseMutex(HANDLE Mutex);
```

user.dll · export `User_ReleaseMutex1` · SDK 10

Releases one hold. The mutex is free when every hold has been released.

### User_MutexHeld

```c
BOOL User_MutexHeld(HANDLE Mutex);
```

user.dll · export `User_MutexHeld1` · SDK 10

Whether anything holds the mutex at this moment, without taking it.

### User_DestroyMutex

```c
VOID User_DestroyMutex(HANDLE Mutex);
```

user.dll · export `User_DestroyMutex1` · SDK 10

Gives the handle back, releasing the mutex first if the caller holds it. The
named object itself goes when nothing holds a handle to it.

---

## Events

Something that was started has finished. An auto-reset event clears on the
read that sees it, so one completion tells exactly one reader; a manual one
stays set until cleared, which is what a state wants. Poll is what almost
every application should use: it asks once and carries on with its frame.

### User_CreateEvent

```c
HANDLE User_CreateEvent(PSTR Name, BOOL AutoReset);
```

user.dll · export `User_CreateEvent1` · SDK 10

Opens the event with this name, creating it clear if nothing has.
`AutoReset` is used only when this call creates it.

- **Returns** the handle, or `HANDLE_INVALID` with `ERROR_OUT_OF_RESOURCES`.

### User_SetEvent

```c
VOID User_SetEvent(HANDLE Event);
```

user.dll · export `User_SetEvent1` · SDK 10

Sets the event.

### User_ClearEvent

```c
VOID User_ClearEvent(HANDLE Event);
```

user.dll · export `User_ClearEvent1` · SDK 10

Clears the event.

### User_PollEvent

```c
BOOL User_PollEvent(HANDLE Event);
```

user.dll · export `User_PollEvent1` · SDK 10

Whether the event is set, without waiting. An auto-reset event that answers
`TRUE` is cleared by the asking.

### User_WaitEvent

```c
BOOL User_WaitEvent(HANDLE Event, WORD32 TimeoutMs);
```

user.dll · export `User_WaitEvent1` · SDK 10

Waits up to `TimeoutMs` (never more than `USER_WAIT_LONGEST`) for the event to
be set. For an event set by the kernel or a device; an event set by another
application cannot be set while this one waits. **Returns** `FALSE` on expiry.

### User_DestroyEvent

```c
VOID User_DestroyEvent(HANDLE Event);
```

user.dll · export `User_DestroyEvent1` · SDK 10

Gives the handle back.

---

## Memory

The heap is the system's, and every block is recorded against the application
that was running when it was allocated. When an application ends, whatever it
still holds is freed with it, so a leak lasts as long as the application and
no longer. Task Manager shows the total.

### User_Allocate

```c
PVOID User_Allocate(WORD32 Size);
```

user.dll · export `User_Allocate1` · SDK 10

Allocates `Size` bytes. The block is not cleared.

- **Returns** the block, or `NULL` with `ERROR_MEMORY_FAILURE` set.

### User_Free

```c
VOID User_Free(PVOID Pointer);
```

user.dll · export `User_Free1` · SDK 10

Frees a block. An application frees only what it allocated: a pointer that is
not one of its blocks is refused with `ERROR_INVALID_ARGUMENT`, and `NULL` is
refused the same way.

### User_Reallocate

```c
PVOID User_Reallocate(PVOID Existing, WORD32 Size);
```

user.dll · export `User_Reallocate1` · SDK 10

Changes a block's size, keeping its contents up to the smaller of the two
sizes. The block may move.

- **Returns** the block, or `NULL` with `ERROR_MEMORY_FAILURE` set, in which
  case `Existing` is untouched and still the caller's.

---

## Time

The machine's clock counts seconds since 1970, and keeps local time rather
than UTC, so a reading turns straight into the date the taskbar shows.

```c
typedef struct _UTM {
    unsigned short Year;
    unsigned char Month, Day, Hour, Minute, Second;
} UTM, *PUTM;
```

### User_GetKTime

```c
WORD32 User_GetKTime(PWORD32 Opt_OutTime);
```

user.dll · export `User_GetKTime1` · SDK 10

The clock, in seconds since 1970.

- `Opt_OutTime` — receives the same value when not `NULL`.
- **Returns** the time.

### User_LocalizeTime

```c
VOID User_LocalizeTime(WORD32 Time, PUTM OutTime);
```

user.dll · export `User_LocalizeTime1` · SDK 10

Breaks a time in seconds into a date and a time of day, applying the zone and
daylight saving set in System Control. A time of zero is refused with
`ERROR_INVALID_ARGUMENT` and `OutTime` is left alone, since zero is a file
that never recorded one.

### User_ZuluTime

```c
VOID User_ZuluTime(WORD32 Time, PUTM OutPut);
```

user.dll · export `User_ZuluTime1` · SDK 10

Breaks a time into its parts the same way as
[`User_LocalizeTime`](#user_localizetime). In SDK 10 the two answer the same:
the machine keeps one clock, in local time.

### User_GetMsTime

```c
WORD32 User_GetMsTime(VOID);
```

user.dll · export `User_GetMsTime1` · SDK 10

A millisecond count from an arbitrary point, for measuring rather than for
telling the time. It wraps after about seven weeks, so the difference between
two readings is meaningful and a single reading is not.

---

## Keyboard and pointer

A key code names a key, not a character: a letter's code is its capital and a
digit's is the digit, whatever Shift says, so `'A'` is 0x41 and `'1'` is 0x31.
The mouse buttons share the space at the bottom, as `MSGKEY_LBUTTON` (1) and
`MSGKEY_RBUTTON` (2). The rest:

| Key | Code |
|---|---|
| Backspace, Tab | 8, 9 |
| Return (either) | 13 |
| Shift | 16 left, 161 right |
| Control | 17 left, 163 right |
| Alt | 18 left, 165 right |
| Caps Lock | 20 |
| Escape | 27 |
| Space | 32 |
| Page Up, Page Down | 33, 34 |
| End, Home | 35, 36 |
| Left, Up, Right, Down | 37, 38, 39, 40 |
| Insert, Delete | 45, 46 |
| F1 to F12 | 112 to 123 |
| semicolon, equals, comma, minus, full stop, slash | 186 to 191 |
| backquote | 192 |
| open bracket, backslash, close bracket, quote | 219 to 222 |

The keypad depends on the keyboard. A USB keyboard gives the keypad's own
digits, 96 to 105; a PS/2 keyboard gives the navigation codes above, as if Num
Lock were off. The character a key types arrives separately, as
[`MSG_CHAR`](messages.md).

There are two kinds of question. *Is it down now* is sampled once a frame.
*Did it go down since last frame* is collected in the interrupt, so a key
struck and released inside one frame is still seen. A window procedure is
told about input as messages ([`MSG_KEYDOWN`, `MSG_CHAR`](messages.md)) and
rarely needs any of these.

### User_GetKeyState

```c
BOOL User_GetKeyState(WORD8 KeyCode);
```

user.dll · export `User_GetKeyState1` · SDK 10

Whether a key is held down this frame.

### User_GetKeyboardState

```c
VOID User_GetKeyboardState(PWORD8 Buffer);
```

user.dll · export `User_GetKeyboardState1` · SDK 10

All 256 key states in one call, one byte each, non-zero for down. `Buffer`
must hold 256 bytes. One call a frame instead of 256.

### User_WasKeyPressed

```c
BOOL User_WasKeyPressed(WORD8 KeyCode);
```

user.dll · export `User_WasKeyPressed1` · SDK 10

Whether a key went down since the last frame. Valid for one frame: ask every
frame or not at all.

### User_WasKeyReleased

```c
BOOL User_WasKeyReleased(WORD8 KeyCode);
```

user.dll · export `User_WasKeyReleased1` · SDK 10

Whether a key came up since the last frame.

### User_WasButtonPressed

```c
BOOL User_WasButtonPressed(WORD8 Button);
```

user.dll · export `User_WasButtonPressed1` · SDK 10

Whether a mouse button went down since the last frame. `Button` is
`MSGKEY_LBUTTON` or `MSGKEY_RBUTTON`.

### User_WasButtonReleased

```c
BOOL User_WasButtonReleased(WORD8 Button);
```

user.dll · export `User_WasButtonReleased1` · SDK 10

Whether a mouse button came up since the last frame.

### User_GetButtonState

```c
BOOL User_GetButtonState(WORD8 Button);
```

user.dll · export `User_GetButtonState1` · SDK 10

Whether one mouse button is held: `MSGKEY_LBUTTON` (1) or `MSGKEY_RBUTTON`
(2). Any other value answers `FALSE`.

### User_GetClickState

```c
BOOL User_GetClickState(WORD8 ClickCode);
```

user.dll · export `User_GetClickState1` · SDK 10

Whether *any* mouse button is held, whatever `ClickCode` says. Kept for
applications written before [`User_GetButtonState`](#user_getbuttonstate),
which is the one to use.

### User_GetMousePosition

```c
VOID User_GetMousePosition(PWORD16 MouseX, PWORD16 MouseY);
```

user.dll · export `User_GetMousePosition1` · SDK 10

Where the pointer is, in screen coordinates, both halves from the same frame.
Either pointer may be `NULL`.

### User_GetWheelTotal

```c
long User_GetWheelTotal(VOID);
```

user.dll · export `User_GetWheelTotal1` · SDK 10

Wheel notches since the machine started, positive away from the user. A
total rather than a change, so a caller keeps the last reading and subtracts,
and several readers each see every notch.

### User_InputLost

```c
WORD32 User_InputLost(VOID);
```

user.dll · export `User_InputLost1` · SDK 10

How many key and button transitions were dropped between this frame and the
last. Zero unless the machine stopped for longer than the input ring is deep.

### User_BufferKeyState

```c
VOID User_BufferKeyState(WORD8 KeyCode, BOOL State);
```

user.dll · export `User_BufferKeyState1` · SDK 10

Sets the state the system holds for a key, as though it had been pressed or
released. For an application that has acted on a key and wants it seen as up,
so it is not acted on again.

### User_BufferClickState

```c
VOID User_BufferClickState(WORD8 ClickCode, BOOL State);
```

user.dll · export `User_BufferClickState1` · SDK 10

Sets the state the system holds for a mouse button. `ClickCode` here counts
from zero (0 is the left button). The common use is
`User_BufferClickState(0, FALSE)` after acting on a click, so the press that
opened something does not also land in it.

### User_BufferMousePosition

```c
VOID User_BufferMousePosition(WORD16 MouseX, WORD16 MouseY);
```

user.dll · export `User_BufferMousePosition1` · SDK 10

Sets where the system believes the pointer is, until the next movement of the
real one.

---

## Files

Paths are absolute, separated by `/`, and matched without regard to case:
`/users/default/desktop/notes.txt`. The volume is BDFS3; see
[folders.h](types.md#folders) for where things live.

A file handle has a cursor, which reads and writes start at and move. Sizes
and positions are 64-bit throughout: the calls ending in `2` take and return
them at full width, and the three without it are the same calls seen through
a 32-bit window, kept for applications written before.

```c
#define USER_FILEACCESS_READ   1
#define USER_FILEACCESS_WRITE  2
#define USER_FILEACCESS_DELETE 4
#define USER_FILEACCESS_CREATE 8

#define USER_FILECSRPOSFROM_HEAD 0   /* from the start */
#define USER_FILECSRPOSFROM_HERE 1   /* from the cursor */
#define USER_FILECSRPOSFROM_TAIL 2   /* from the end */

#define USER_FILETYPE_NONE 0
#define USER_FILETYPE_FILE 1
#define USER_FILETYPE_DIR  2

typedef struct _USER_FILEINFO {
    WORD32 Type;        /* USER_FILETYPE_* */
    WORD32 Flags;       /* USER_ATTR_* */
    WORD64 Size;
    WORD64 Created;     /* seconds since 1970, machine time; 0 if never recorded */
    WORD64 Modified;
    WORD32 Owner;       /* reserved */
    WORD32 Mode;        /* reserved */
    WORD32 Frame;       /* reserved */
    WORD32 Accessed;
} USER_FILEINFO, *PUSER_FILEINFO;
```

**The two locks.** A file marked read-only (`USER_ATTR_READONLY`) refuses every
write. A file marked system (`USER_ATTR_SYSTEM`) is the machine's own: an
application cannot write, delete or rename it. Neither is a permission, and
[`User_SetAttributes`](#user_setattributes) is how either is taken off. A
refused change sets `ERROR_ACCESS_DENIED`.

### User_OpenFile

```c
HANDLE User_OpenFile(PSTR FilePath, WORD16 Permissions);
```

user.dll · export `User_OpenFile1` · SDK 10

Opens a file, or a folder, by path. The cursor starts at zero.

- `Permissions` — `USER_FILEACCESS_*` together. With `USER_FILEACCESS_CREATE`
  a file that is not there is made empty; without it, a missing file fails.
  Asking for `USER_FILEACCESS_WRITE` on a locked file fails here rather than at
  the first write.
- **Returns** the handle, or `HANDLE_INVALID` with `ERROR_INVALID_PATH`,
  `ERROR_PATH_NOT_FOUND` or `ERROR_ACCESS_DENIED` set.

Close it with [`User_DestroyHandle`](#user_destroyhandle).

### User_GetFileInfo

```c
BOOL User_GetFileInfo(HANDLE File, PUSER_FILEINFO Out);
```

user.dll · export `User_GetFileInfo1` · SDK 10

What a file is, apart from what is in it: its type, size, times and
attributes. **Returns** `FALSE` for a bad handle or a `NULL` `Out`.

### User_ReadFile

```c
WORD32 User_ReadFile(HANDLE File, PVOID Buffer, WORD32 BufferSize,
                     WORD32 BytesToRead);
```

user.dll · export `User_ReadFile1` · SDK 10

Reads from the cursor into `Buffer`, at most the smaller of `BytesToRead` and
`BufferSize`, and moves the cursor by what it read. A run of reads walks the
file.

- **Returns** how many bytes were read, which is less than asked at the end
  of the file and zero past it; `0xFFFFFFFF` for a bad handle or a `NULL`
  buffer.

### User_WriteFile

```c
WORD32 User_WriteFile(HANDLE File, PVOID Buffer, WORD32 BytesToWrite);
```

user.dll · export `User_WriteFile1` · SDK 10

Writes at the cursor, extending the file if the cursor is at or past its end,
and moves the cursor by what it wrote.

- **Returns** how many bytes were written: zero for a locked file, with
  `ERROR_ACCESS_DENIED`; `0xFFFFFFFF` for a bad handle or a `NULL` buffer.

### User_UpdateFileCursor2

```c
WORD64 User_UpdateFileCursor2(HANDLE File, WORD8 CursorMode, WORD64 NewPos);
```

user.dll · export `User_UpdateFileCursor2` · SDK 10

Moves the cursor: to `NewPos` from the start, by `NewPos` from where it is, or
to `NewPos` past the end, by `CursorMode`. A position past the end is allowed,
and is where a write extends the file.

- **Returns** the new position, or `USER_FILECSR_ERROR` for a bad handle.

### User_UpdateFileCursor

```c
WORD32 User_UpdateFileCursor(HANDLE File, WORD8 CursorMode, WORD32 NewPos);
```

user.dll · export `User_UpdateFileCursor1` · SDK 10

The 32-bit form of [`User_UpdateFileCursor2`](#user_updatefilecursor2). In
the relative modes `NewPos` is signed, so a backward move is a negative
number. **Returns** `0xFFFFFFFF` for a bad handle or a position past 4 GB.

### User_GetFileSize2

```c
WORD64 User_GetFileSize2(HANDLE File);
```

user.dll · export `User_GetFileSize2` · SDK 10

The size of the file in bytes, or `USER_FILESIZE_ERROR` for a bad handle.

### User_GetFileSize

```c
WORD32 User_GetFileSize(HANDLE File);
```

user.dll · export `User_GetFileSize1` · SDK 10

The 32-bit form. **Returns** `0xFFFFFFFF` for a bad handle or a file of 4 GB
or more.

### User_TruncateFile2

```c
BOOL User_TruncateFile2(HANDLE File, WORD64 Size);
```

user.dll · export `User_TruncateFile2` · SDK 10

Sets the file's length, cutting it short or extending it. The cursor is not
moved. **Returns** `FALSE` for a bad handle or a locked file.

### User_TruncateFile

```c
BOOL User_TruncateFile(HANDLE File, WORD32 Size);
```

user.dll · export `User_TruncateFile1` · SDK 10

The 32-bit form of [`User_TruncateFile2`](#user_truncatefile2).

### User_GetFileName

```c
WORD32 User_GetFileName(HANDLE File, PSTR Buffer, WORD32 BufferSize);
```

user.dll · export `User_GetFileName1` · SDK 10

The whole path of what a handle is open on, terminated, cut short to fit.
**Returns** its length, 0 when the path could not be found, or `0xFFFFFFFF`
for a bad handle or buffer.

### User_DeleteFile

```c
VOID User_DeleteFile(HANDLE File);
```

user.dll · export `User_DeleteFile1` · SDK 10

Deletes the file a handle is open on, and gives the handle back whether or
not the delete happened. Refused with `ERROR_ACCESS_DENIED` while another
handle has the file open, or when it is locked.

### User_DeletePath

```c
BOOL User_DeletePath(PSTR Path);
```

user.dll · export `User_DeletePath1` · SDK 10

Deletes a file, or an empty folder, by path. **Returns** `FALSE` when there
is nothing there, the path is locked, or a folder is not empty.

### User_DeleteTree

```c
BOOL User_DeleteTree(PSTR Path);
```

user.dll · export `User_DeleteTree1` · SDK 10

Deletes a path and everything under it. The whole tree is checked first, and
if anything in it is locked nothing is deleted. Trees deeper than
`USER_DELETETREE_MAXDEPTH` (16) are refused with `ERROR_INVALID_PATH`.

### User_RenameFile

```c
BOOL User_RenameFile(PSTR Path, PSTR NewPath);
```

user.dll · export `User_RenameFile1` · SDK 10

Renames or moves a file or folder. A read-only file may be renamed, since
none of it changes; a system file may not, and an existing system file is not
renamed over either.

### User_CopyFile

```c
BOOL User_CopyFile(PSTR Path, PSTR NewPath);
```

user.dll · export `User_CopyFile1` · SDK 10

Copies a file, replacing whatever is at `NewPath` unless that is locked. A
folder is refused. Moved a block at a time, so any size of file costs the
same memory.

### User_ValidateFilePath

```c
BOOL User_ValidateFilePath(PSTR Path);
```

user.dll · export `User_ValidateFilePath1` · SDK 10

Whether a path is well formed — it starts with `/`, it is shorter than 1024
characters, and it has no empty components (`//`) — without asking whether
anything is there. One trailing `/` is allowed, and names a folder.

---

## Attributes

```c
#define USER_ATTR_NONE     0x00
#define USER_ATTR_READONLY 0x01
#define USER_ATTR_HIDDEN   0x02
#define USER_ATTR_SYSTEM   0x04

#define USER_ATTRLOCKED(a)    /* read-only or system: refuses changes */
#define USER_ATTRCONCEALED(a) /* hidden or system: left out of listings */
```

Hidden asks a listing not to show a file. System is read-only and hidden
together, as an application sees it.

### User_GetAttributes

```c
WORD32 User_GetAttributes(PSTR Path);
```

user.dll · export `User_GetAttributes1` · SDK 10

What a path carries, or `USER_ATTR_NONE` when nothing is there.

### User_SetAttributes

```c
BOOL User_SetAttributes(PSTR Path, WORD32 Attributes);
```

user.dll · export `User_SetAttributes1` · SDK 10

Replaces a path's attributes whole: read them, change the bit meant, write
them back. Never refused for a lock, since this is what takes a lock off.

---

## Folders

### User_OpenDirectory

```c
HANDLE User_OpenDirectory(PSTR Path);
```

user.dll · export `User_OpenDirectory1` · SDK 10

Opens a folder to list it. **Returns** `HANDLE_INVALID` with
`ERROR_PATH_NOT_FOUND`, or `ERROR_INVALID_PATH` when the path is a file.

### User_ReadDirectory

```c
BOOL User_ReadDirectory(HANDLE Directory, WORD32 Index, PSTR NameOut,
                        WORD32 NameMax, PWORD32 TypeOut);
```

user.dll · export `User_ReadDirectory1` · SDK 10

One entry by position. Walk upward from zero until it answers `FALSE`.

- `NameOut`, `NameMax` — receives the leaf name, terminated.
  `USER_NAMEMAX` (256) always fits.
- `TypeOut` — receives a `USER_FILETYPE_*` when not `NULL`.

```c
HANDLE Folder = User_OpenDirectory("/users/default/desktop");
char Name[USER_NAMEMAX];
WORD32 Type;

for (WORD32 i = 0; User_ReadDirectory(Folder, i, Name, sizeof(Name), &Type); i++)
    ...;

User_DestroyHandle(Folder);
```

### User_CountDirectory

```c
WORD32 User_CountDirectory(HANDLE Directory);
```

user.dll · export `User_CountDirectory1` · SDK 10

How many entries a folder holds, or 0 for a bad handle.

### User_CreateDirectory

```c
BOOL User_CreateDirectory(PSTR Path);
```

user.dll · export `User_CreateDirectory1` · SDK 10

Makes a folder. The folder above it must already be there.

### User_FlushFileSystem

```c
VOID User_FlushFileSystem(VOID);
```

user.dll · export `User_FlushFileSystem1` · SDK 10

Writes out anything the volume is holding back. The system flushes on its
own; this is for before something that will not come back.

### User_GetVolumeSpace

```c
BOOL User_GetVolumeSpace(PSTR Path, PWORD64 Total, PWORD64 Free);
```

user.dll · export `User_GetVolumeSpace1` · SDK 10

The size of the volume a path is on and how much of it is free, in bytes.
`"/"` names the volume the machine started from. Both outputs are zeroed
first, and stay zero when it answers `FALSE`.

---

## Names as they are shown

### User_HideExtensions

```c
BOOL User_HideExtensions(VOID);
```

user.dll · export `User_HideExtensions1` · SDK 10

Whether Folder Options asks for extensions to be left off in listings. Read
from the registry each time, so ask while building a list, not while painting
one.

### User_DisplayName

```c
VOID User_DisplayName(PSTR Name, BOOL IsDirectory, PSTR Out, WORD32 Size);
```

user.dll · export `User_DisplayName1` · SDK 10

One leaf name as somebody should see it. The extension comes off when
Folder Options says so — never from a folder, and never when the dot is the
first character, so `.config` keeps its name. A link's `.bal` always comes
off.

---

## Sound

A sound is a resource, played by name; it is short, resident, and finishes on
its own, so there is nothing to hold or free. The system's own are
`Sound_Startup`, `Sound_Shutdown`, `Sound_Error` and `Sound_Notification`. An
application plays one of its own by carrying it ([`[sounds]` in
app.ini](../guide/project-file.md#sounds)) and registering it once with
[`User_RegisterSound`](modres.md#user_registersound).

### User_PlaySound

```c
BOOL User_PlaySound(PSTR Name);
```

user.dll · export `User_PlaySound1` · SDK 10

Plays a registered sound. **Returns** `FALSE` when there is no sound device,
nothing registered under the name, or every voice is busy; none of the three
is worth acting on.

### User_SoundReady

```c
BOOL User_SoundReady(VOID);
```

user.dll · export `User_SoundReady1` · SDK 10

Whether there is a sound device, for an application that would rather leave
out a control than offer one that does nothing.

---

## The system's version

Each answers with a string the system owns, which stays valid and must not be
written or freed.

### UserVersion_GetName

```c
PSTR UserVersion_GetName(VOID);
```

user.dll · export `UserVersion_GetName1` · SDK 10

The system's name: `"BoltOS"`.

### UserVersion_GetProductName

```c
PSTR UserVersion_GetProductName(VOID);
```

user.dll · export `UserVersion_GetProductName1` · SDK 10

The product name as the desktop shows it, with the release stage:
`"N. B. Wooten(R) BoltOS(TM) Beta 1"`.

### UserVersion_GetBuildString

```c
PSTR UserVersion_GetBuildString(VOID);
```

user.dll · export `UserVersion_GetBuildString1` · SDK 10

The whole build string, as in `BoltOS 2.0 Build 415.0.sdkv10.261003-2100`.

### UserVersion_GetBranchString

```c
PSTR UserVersion_GetBranchString(VOID);
```

user.dll · export `UserVersion_GetBranchString1` · SDK 10

The branch the build came from: `"sdkv10"`.

### UserVersion_GetCopyright

```c
PSTR UserVersion_GetCopyright(VOID);
```

user.dll · export `UserVersion_GetCopyright1` · SDK 10

The system's copyright line: `"Copyright (c) 2023-2026 Noah Wooten"`.

### UserVersion_GetShortVersion

```c
PSTR UserVersion_GetShortVersion(VOID);
```

user.dll · export `UserVersion_GetShortVersion1` · SDK 10

The version with the build number: `"Version 2.0 (Build 415)"`. A Debug
build of the system adds the revision, the branch and `(Checked / Debug)`.

### UserVersion_GetSuperShortVersion

```c
PSTR UserVersion_GetSuperShortVersion(VOID);
```

user.dll · export `UserVersion_GetSuperShortVersion1` · SDK 10

The shortest form of the version, for a narrow place: `"Version 2.0"`.

### UserVersion_GetMajor

```c
WORD32 UserVersion_GetMajor(VOID);
```

user.dll · export `UserVersion_GetMajor1` · SDK 10

The major version: 2.

### UserVersion_GetMinor

```c
WORD32 UserVersion_GetMinor(VOID);
```

user.dll · export `UserVersion_GetMinor1` · SDK 10

The minor version: 0.

### UserVersion_GetBuild

```c
WORD32 UserVersion_GetBuild(VOID);
```

user.dll · export `UserVersion_GetBuild1` · SDK 10

The build number: 415 for the system this SDK targets.

### UserVersion_GetRevision

```c
WORD32 UserVersion_GetRevision(VOID);
```

user.dll · export `UserVersion_GetRevision1` · SDK 10

The revision within the build.

### UserVersion_GetDate

```c
WORD32 UserVersion_GetDate(VOID);
```

user.dll · export `UserVersion_GetDate1` · SDK 10

The day it was built, as a number written `YYMMDD`: 261003 is 3 October 2026.

### UserVersion_GetTime

```c
WORD32 UserVersion_GetTime(VOID);
```

user.dll · export `UserVersion_GetTime1` · SDK 10

The time of day it was built, as `HHMM`.

### UserVersion_GetSku

```c
PSTR UserVersion_GetSku(VOID);
```

user.dll · export `UserVersion_GetSku1` · SDK 10

The edition: `"ClientSku"` for every build this SDK targets.

### UserVersion_GetArch

```c
PSTR UserVersion_GetArch(VOID);
```

user.dll · export `UserVersion_GetArch1` · SDK 10

The processor architecture the system was built for: `"x64native"`.

To know which SDK the system provides, compare against `BOLTSDK_APIV_LATEST`
at build time; an application never runs on a system older than the SDK it
names, since the system refuses it.

---

## Miscellaneous

### User_GetLastError

```c
WORD32 User_GetLastError(VOID);
```

user.dll · export `User_GetLastError1` · SDK 10

Why the last call that failed failed: an `ERROR_*` code. Not cleared by a
call that succeeds, so read it straight after the failure it explains.

### User_InsecureHashString

```c
WORD32 User_InsecureHashString(PSTR Str);
```

user.dll · export `User_InsecureHashString1` · SDK 10

A 32-bit hash of a string (djb2). Fast and stable from one build to the next,
and good for a table; not for anything that has to resist somebody choosing
the input.

---

## Services

A service is a set of callbacks a module registers to be run on the system's
clock whether or not any window is open — the clock application's alarm is
one. An application describes its services with `MODRES_RSRCTYPE_SERVICE`
resources (`RSRCTYPE_SERVICE` in user.h). These calls find and control the
services that are registered.

```c
#define SVCSTATE_ERROR   0
#define SVCSTATE_PAUSED  1
#define SVCSTATE_STOPPED 2
#define SVCSTATE_FAILURE 3
#define SVCSTATE_RUNNING 4
```

### UserService_GetCount

```c
WORD32 UserService_GetCount(VOID);
```

user.dll · export `UserService_GetCount1` · SDK 10

How many services are registered. Identifiers are table positions and the
table may have gaps, so walk with [`UserService_GetName`](#userservice_getname)
up to 64.

### UserService_GetName

```c
PSTR UserService_GetName(WORD32 Id);
```

user.dll · export `UserService_GetName1` · SDK 10

The name of the service at a position, an empty string for an empty slot, or
`NULL` past the end. The string is the system's.

### UserService_GetById

```c
HANDLE UserService_GetById(WORD32 Id);
```

user.dll · export `UserService_GetById1` · SDK 10

A handle to the service at a position, or `HANDLE_INVALID` with
`ERROR_INVALID_SVC_ID`.

### UserService_GetByName

```c
HANDLE UserService_GetByName(PSTR Name);
```

user.dll · export `UserService_GetByName1` · SDK 10

A handle to the service with this name, or `HANDLE_INVALID` with
`ERROR_INVALID_SVC_NAME`.

### UserService_Start

```c
VOID UserService_Start(HANDLE Service);
```

user.dll · export `UserService_Start1` · SDK 10

Starts a stopped service, running its initialisation, or lets a paused one go
on. Sets `ERROR_SVC_ALREADY_RUN` when it is running already.

### UserService_Stop

```c
VOID UserService_Stop(HANDLE Service);
```

user.dll · export `UserService_Stop1` · SDK 10

Stops a running or paused service, running its shutdown once. Sets
`ERROR_SVC_ALREADY_STOP` when it is stopped already.

### UserService_Pause

```c
VOID UserService_Pause(HANDLE Service);
```

user.dll · export `UserService_Pause1` · SDK 10

Holds a running service: its clock is not called until it is started again.
Sets `ERROR_SVC_ALREADY_STOP` for a stopped one and `ERROR_SVC_ALREADY_PAUSE`
for a paused one.

### UserService_GetState

```c
BOOL UserService_GetState(HANDLE Service);
```

user.dll · export `UserService_GetState1` · SDK 10

What a service is doing: `SVCSTATE_RUNNING`, `SVCSTATE_PAUSED`,
`SVCSTATE_STOPPED`, or `SVCSTATE_ERROR` for a bad handle. Declared `BOOL`,
and answers one of those values.

### UserService_SetStartupState

```c
VOID UserService_SetStartupState(HANDLE Service, BOOL State);
```

user.dll · export `UserService_SetStartupState1` · SDK 10

Whether a service starts with the machine. Recorded in the registry. Only
the shell and System Control may change it: from an application's own code
it does nothing.

---

## The command line

A session is a conversation with the same command line telnet and the
Terminal application use. Output is collected rather than pushed, so an
application drains it on its own clock.

```c
#define USERCONSOLE_CLOSE 0x01   /* the session is finished */
#define USERCONSOLE_CLEAR 0x02   /* clear the screen */
```

### UserConsole_Open

```c
HANDLE UserConsole_Open(VOID);
```

user.dll · export `UserConsole_Open1` · SDK 10

Starts a session. **Returns** its handle, or `HANDLE_INVALID` with
`ERROR_OUT_OF_RESOURCES`.

### UserConsole_Close

```c
VOID UserConsole_Close(HANDLE Console);
```

user.dll · export `UserConsole_Close1` · SDK 10

Ends a session and gives the handle back.

### UserConsole_Submit

```c
VOID UserConsole_Submit(HANDLE Console, PSTR Line);
```

user.dll · export `UserConsole_Submit1` · SDK 10

Runs one line, as though it had been typed and Return pressed. What it
produces is read with [`UserConsole_Collect`](#userconsole_collect), which may
take more than one call and more than one frame.

### UserConsole_Collect

```c
WORD32 UserConsole_Collect(HANDLE Console, PSTR Buffer, WORD32 Capacity);
```

user.dll · export `UserConsole_Collect1` · SDK 10

Takes what the session has produced since the last call. **Returns** how many
bytes, which may be zero. The buffer is not terminated.

### UserConsole_Busy

```c
BOOL UserConsole_Busy(HANDLE Console);
```

user.dll · export `UserConsole_Busy1` · SDK 10

Whether the last line is still being answered. A command that waits on the
network finishes in a later frame than the one that started it.

### UserConsole_TakeFlags

```c
WORD32 UserConsole_TakeFlags(HANDLE Console);
```

user.dll · export `UserConsole_TakeFlags1` · SDK 10

What a command asked of whoever shows the session — `USERCONSOLE_CLEAR`,
`USERCONSOLE_CLOSE` — read once and cleared.

### UserConsole_Prompt

```c
PSTR UserConsole_Prompt(VOID);
```

user.dll · export `UserConsole_Prompt1` · SDK 10

The prompt every front end draws before a line. The string is the system's.

### UserConsole_Banner

```c
PSTR UserConsole_Banner(VOID);
```

user.dll · export `UserConsole_Banner1` · SDK 10

The greeting a session opens with. The string is the system's.
