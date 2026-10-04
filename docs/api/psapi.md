# psapi.h — processes

`#include <psapi.h>` · exported by **psapi.dll** · link **`psapi.lib`**

A process is a loaded module with an entry point, and this is the only
sanctioned way to make one or end one. Every application that runs — started
from the Actions menu, the desktop, the file explorer, or
[`Shell_OpenFile`](shellapi.md#shell_openfile) — is a process made here, with
its own copy of its image and its own variables, so two copies of one
application do not share anything.

A process is named by an identifier, which is a slot in the system's module
table. Slot zero is a real one, so the call that fails answers `PS_INVALID`
(`0xFFFFFFFF`), not zero. The table has 16 slots, and the system's own modules
use several of them.

```c
WORD32 Id = Ps_CreateProcess("/programs/myapp/myapp.bxf");

if (Id != PS_INVALID)
    Ps_StartProcess(Id);
```

Creating and starting are separate so a caller can look at the process between
the two. [`Shell_ExecuteEx`](shellapi.md#shell_executeex) does both and also
gives an application that opens no window one described by a link; most
applications want that, or `Shell_OpenFile`, rather than these.

---

### Ps_CreateProcess

```c
WORD32 Ps_CreateProcess(PSTR Path);
```

psapi.dll · export `Ps_CreateProcess1` · SDK 10

Loads the application at `Path` and gives it a slot, without running it.

- **Returns** the process identifier, or `PS_INVALID` when the file cannot be
  read or loaded, its imports cannot all be bound, the table is full, or it was
  built against a newer Bolt SDK than the system provides
  ([`UserRes_SdkVersion`](modres.md#userres_sdkversion) says which).

### Ps_StartProcess

```c
BOOL Ps_StartProcess(WORD32 Process);
```

psapi.dll · export `Ps_StartProcess1` · SDK 10

Runs the entry point, `__modmain`, with the process as the running module, so
everything it allocates and every window it opens is billed to it. Starting a
process that is running already enters it again, which is how it opens a
second window.

### Ps_KillProcess

```c
BOOL Ps_KillProcess(WORD32 Process, BOOL Graceful);
```

psapi.dll · export `Ps_KillProcess1` · SDK 10

Ends a process and releases everything the system records against it: its
memory, its handles, its windows. `Graceful` calls `__modhalt` first, which an
application expects to save from; without it the process is dropped where it
stands. Only an application can be killed; the system's own modules refuse.

### Ps_SuspendProcess

```c
BOOL Ps_SuspendProcess(WORD32 Process);
```

psapi.dll · export `Ps_SuspendProcess1` · SDK 10

Stops calling the process's clock. Its windows stay on screen showing what
they last drew. Applications only.

### Ps_ResumeProcess

```c
BOOL Ps_ResumeProcess(WORD32 Process);
```

psapi.dll · export `Ps_ResumeProcess1` · SDK 10

Starts calling it again.

### Ps_IsSuspended

```c
BOOL Ps_IsSuspended(WORD32 Process);
```

psapi.dll · export `Ps_IsSuspended1` · SDK 10

### Ps_IsProcess

```c
BOOL Ps_IsProcess(WORD32 Process);
```

psapi.dll · export `Ps_IsProcess1` · SDK 10

Whether an identifier names a loaded process. The table has gaps, so this is
what a walk asks of each slot.

### Ps_IsApplication

```c
BOOL Ps_IsApplication(WORD32 Process);
```

psapi.dll · export `Ps_IsApplication1` · SDK 10

Whether a process is an application rather than one of the system's own
modules: what to ask before offering to end or suspend it.

### Ps_GetCurrentProcess

```c
WORD32 Ps_GetCurrentProcess(VOID);
```

psapi.dll · export `Ps_GetCurrentProcess1` · SDK 10

The calling application's own identifier.

### Ps_GetProcessMax

```c
WORD32 Ps_GetProcessMax(VOID);
```

psapi.dll · export `Ps_GetProcessMax1` · SDK 10

How many slots there are, so a caller can walk them:

```c
for (WORD32 Id = 0; Id < Ps_GetProcessMax(); Id++)
    if (Ps_IsProcess(Id) && Ps_IsApplication(Id))
        ...;
```

### Ps_GetProcessPath

```c
BOOL Ps_GetProcessPath(WORD32 Process, PSTR Out, WORD32 Size);
```

psapi.dll · export `Ps_GetProcessPath1` · SDK 10

The file a process was started from. What it carries lives in that file, so
this is what to pass to
[`Wndmgr_LoadIconFrom`](wndmgr.md#wndmgr_loadiconfrom) to draw another
application's icon.

### Ps_GetProcessBase

```c
PVOID Ps_GetProcessBase(WORD32 Process);
```

psapi.dll · export `Ps_GetProcessBase1` · SDK 10

Where a process's image is loaded, or `NULL`.

### Ps_FindProcessByBase

```c
WORD32 Ps_FindProcessByBase(PVOID Base);
```

psapi.dll · export `Ps_FindProcessByBase1` · SDK 10

The process loaded at a base, or `PS_INVALID`.
