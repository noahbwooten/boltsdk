# shellapi.h — the shell

`#include <shellapi.h>` · exported by **boltshell.dll** (`/sys/core/shell.bxf`)
· link **`boltshell.lib`**

What the shell offers an application: starting programs and opening files,
message boxes, links, the icon and type name a file is shown with, and the
original immediate-mode controls.

---

## Starting things

### Shell_OpenFile

```c
BOOL Shell_OpenFile(PSTR Path);
```

boltshell.dll · export `Shell_OpenFile1` · SDK 10

Opens a path the way a double click does: an application (`.bxf`) is started;
a link (`.bal`) is followed; a folder opens a file explorer at it; a document
goes to whatever reads it — `.txt` and `.log` to Notepad, `.zip` and `.tar` to
the archive utility, `.bxi` to the installer. **Returns** `FALSE` when nothing
opens it, which is the caller's to report.

### Shell_OpenFileWith

```c
BOOL Shell_OpenFileWith(PSTR AppPath, PSTR DocumentPath);
```

boltshell.dll · export `Shell_OpenFileWith1` · SDK 10

Starts an application and hands it a document, for a caller that chooses the
application itself. The document arrives through the application's
`__modopen(PSTR Path)` export, called after `__modmain`; an application that
does not export it is started with nothing open.

### Shell_ExecuteEx

```c
PVOID Shell_ExecuteEx(PSTR FilePath, PSHELL_LAUNCHINFO Opt_Info,
                      PWORD16 Opt_OutModuleId);
```

boltshell.dll · export `Shell_ExecuteEx1` · SDK 10

Loads an application and runs its entry point.

- `Opt_Info` — how a window should look, for an application that opens none of
  its own; `NULL` lets the application decide.
- `Opt_OutModuleId` — receives the new process's identifier.
- **Returns** where it was loaded, or `NULL` when it could not be started. An
  application built for a newer Bolt SDK than the system provides is refused,
  and the shell tells the user why.

```c
typedef struct _SHELL_LAUNCHINFO {
    WORD32 X, Y, W, H;
    char Title[64];         /* copied */
    char IconName[32];      /* a picture the application carries */
    WORD32 Flags;           /* WNDMGRHNDFLAGS_* */
} SHELL_LAUNCHINFO;
```

### Shell_Execute

```c
PVOID Shell_Execute(char* FilePath, WORD16 ObjId, PVOID* NativeModule,
                    WORD16* ModId);
```

boltshell.dll · export `Shell_Execute1` · SDK 10

The older form of [`Shell_ExecuteEx`](#shell_executeex). `ObjId` is not used,
and `*NativeModule` is always set to `NULL`.

---

## Message boxes

A message box is a window of its own and does not wait. It is *polled*: call
it with the same title and message every frame — from a timer, or from a
window procedure — and it answers `SHELL_MSGBOXRSLT_WAIT` until somebody
presses a button, then the button's result, once, and the box is gone.
Calling it again after that puts up a new one.

```c
static BOOL Asking;

case MSG_INTERVAL:
    if (Asking) {
        WORD32 Answer = Shell_MessageBox("My App", "Save changes?",
            SHELL_MSGBOXICON_QSTN, SHELL_MSGBOXBTN_YES | SHELL_MSGBOXBTN_NO);

        if (Answer != SHELL_MSGBOXRSLT_WAIT) {
            Asking = FALSE;
            if (Answer == SHELL_MSGBOXRSLT_YES)
                Save();
        }
    }
    return 0;
```

Text wraps at 80 characters and at newlines.

| Icons | Buttons (or together) | Results |
|---|---|---|
| `SHELL_MSGBOXICON_NONE` | `SHELL_MSGBOXBTN_OK` | `SHELL_MSGBOXRSLT_WAIT` |
| `SHELL_MSGBOXICON_INFO` | `SHELL_MSGBOXBTN_CANCEL` | `SHELL_MSGBOXRSLT_OK` |
| `SHELL_MSGBOXICON_WARN` | `SHELL_MSGBOXBTN_RETRY` | `SHELL_MSGBOXRSLT_CANCEL` |
| `SHELL_MSGBOXICON_EROR` | `SHELL_MSGBOXBTN_ABORT` | `SHELL_MSGBOXRSLT_RETRY` |
| `SHELL_MSGBOXICON_CTCL` | `SHELL_MSGBOXBTN_FAIL` | `SHELL_MSGBOXRSLT_ABORT` |
| `SHELL_MSGBOXICON_QSTN` | `SHELL_MSGBOXBTN_SAVE` | `SHELL_MSGBOXRSLT_FAIL` |
| | `SHELL_MSGBOXBTN_DONTSAVE` | `SHELL_MSGBOXRSLT_SAVE` |
| | `SHELL_MSGBOXBTN_YES` | `SHELL_MSGBOXRSLT_DONTSAVE` |
| | `SHELL_MSGBOXBTN_NO` | `SHELL_MSGBOXRSLT_YES`, `_NO` |

`SHELL_MSGBOXOPT_SILENT` with the buttons keeps the box from making its own
sound, for an alert that is already sounding.

### Shell_MessageBox

```c
WORD32 Shell_MessageBox(PSTR Title, PSTR Message, BYTE Image, WORD16 Options);
```

boltshell.dll · export `Shell_MessageBox1` · SDK 10

Puts up a box, or answers for the one already up with this title and message.
`Image` is a `SHELL_MSGBOXICON_*`; `Options` the buttons. **Returns** a
`SHELL_MSGBOXRSLT_*`. A box closed some other way answers
`SHELL_MSGBOXRSLT_CANCEL`.

### Shell_GetMessageBoxId

```c
WORD32 Shell_GetMessageBoxId(PSTR Title, PSTR Message);
```

boltshell.dll · export `Shell_GetMessageBoxId1` · SDK 10

The identifier of the box up with this title and message, for
[`Shell_GetMessageBoxStatus`](#shell_getmessageboxstatus).

### Shell_GetMessageBoxStatus

```c
WORD32 Shell_GetMessageBoxStatus(WORD32 Id);
```

boltshell.dll · export `Shell_GetMessageBoxStatus1` · SDK 10

A box's answer by identifier: `SHELL_MSGBOXRSLT_WAIT` until a button is
pressed, then the button's result, once, and the box is gone — the same as
asking [`Shell_MessageBox`](#shell_messagebox) again. An identifier for a box
that has gone answers `SHELL_MSGBOXRSLT_WAIT`.

---

## Links

A link (`.bal`) names a file, what it is called where it is shown, and the
icon it is drawn with; opening one opens what it names. The Actions menu, the
desktop and the startup folder are folders of links.

```c
typedef struct _APPLINK {
    WORD32 Magic;           /* APPLINK_MAGIC */
    WORD32 Version;         /* APPLINK_VERSION */
    char Name[64];          /* what it is called */
    char Path[128];         /* what it opens */
    char IconName[32];      /* a picture the target carries; empty for none */
    char Arguments[128];    /* reserved */
    WORD32 X, Y, W, H;      /* a window, for a target that opens none */
    WORD32 WindowFlags;
    WORD32 Flags;           /* reserved */
    WORD32 Reserved[8];
} APPLINK;
```

### Shell_ReadAppLink

```c
BOOL Shell_ReadAppLink(PSTR Path, PAPPLINK Out);
```

boltshell.dll · export `Shell_ReadAppLink1` · SDK 10

Reads a link. A file that is short, or carries a magic or version this build
does not know, reads as absent.

### Shell_WriteAppLink

```c
BOOL Shell_WriteAppLink(PSTR Path, PAPPLINK Link);
```

boltshell.dll · export `Shell_WriteAppLink1` · SDK 10

Writes a link, filling in its magic and version. To put an application in the
Actions menu, write one into `APPLINK_ACTIONS_PATH` (a folder in it is a
submenu); on the desktop, into `FOLDER_DESKTOP`; to start it with the
machine, into `FOLDER_STARTUP`.

---

## How a file is shown

### Shell_IconNameFor

```c
BOOL Shell_IconNameFor(PSTR Path, WORD32 Type, BOOL Empty, PSTR Out,
                       WORD32 Size);
```

boltshell.dll · export `Shell_IconNameFor1` · SDK 10

The name of the picture a path is drawn with, by what it is. `Type` is a
`USER_FILETYPE_*`; `Empty` chooses between the two folder pictures. A link
answers the icon it names, or its target's. **Returns** `FALSE` when nothing
was decided.

### Shell_IconModuleFor

```c
BOOL Shell_IconModuleFor(PSTR Path, PSTR Out, WORD32 Size);
```

boltshell.dll · export `Shell_IconModuleFor1` · SDK 10

For a link, the module that carries the picture its icon name means, to pass
to [`Wndmgr_LoadIconFrom`](wndmgr.md#wndmgr_loadiconfrom). `FALSE` means the
shell's own.

### Shell_TypeNameFor

```c
BOOL Shell_TypeNameFor(PSTR Path, WORD32 Type, PSTR Out, WORD32 Size);
```

boltshell.dll · export `Shell_TypeNameFor1` · SDK 10

What a path is called, as a person would say it: "Folder", "Text Document
(.txt)", "TMP File (.tmp)" for an extension nothing knows.

---

## Windows

### Shell_CloseWindow

```c
void Shell_CloseWindow(HANDLE Window);
```

boltshell.dll · export `Shell_CloseWindow1` · SDK 10

Closes a window, as [`Wndmgr_CloseWindow`](wndmgr.md#wndmgr_closewindow).

### Shell_MinimizeWindow

```c
void Shell_MinimizeWindow(HANDLE Window);
```

boltshell.dll · export `Shell_MinimizeWindow1` · SDK 10

Minimises a window, as [`Wndmgr_MinimizeWindow`](wndmgr.md#wndmgr_minimizewindow).

### Shell_BringForth

```c
VOID Shell_BringForth(HANDLE Window);
```

boltshell.dll · export `Shell_BringForth1` · SDK 10

Gives a window the focus, as [`Wndmgr_BringForth`](wndmgr.md#wndmgr_bringforth).

### Shell_GetColors

```c
PSHELL_COLORS Shell_GetColors(VOID);
```

boltshell.dll · export `Shell_GetColors1` · SDK 10

The shell's palette, as a `SHELL_COLORS` the shell owns; the same colours as
the window manager's, under the shell's names. Read, not written.

### Shell_GetShellRes

```c
PKMCTL_SHELLRES Shell_GetShellRes(VOID);
```

boltshell.dll · export `Shell_GetShellRes1` · SDK 10

The shell's resources as the system keeps them, as
[`Wndmgr_GetShellRes`](wndmgr.md#wndmgr_getshellres).

### Shell_SetClips

```c
VOID Shell_SetClips(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
```

boltshell.dll · export `Shell_SetClips1` · SDK 10

The shell's form of [`Wndmgr_SetClips`](wndmgr.md#wndmgr_setclips), for the
immediate-mode controls below.

### Shell_GetWindowData

```c
VOID Shell_GetWindowData(PWORD16 MouseX, PWORD16 MouseY, PBOOL LeftButton);
```

boltshell.dll · export `Shell_GetWindowData1` · SDK 10

The pointer as the window the shell is drawing was handed it, for the
immediate-mode controls below.

---

## The shell's immediate-mode controls

The shell's copy of the original controls, with the same contracts as their
`CmnCtl_` namesakes in [cmnctl.h](cmnctl.md). New code uses the
[retained controls](cmnctl2.md).

### ShellUi_DrawButton

```c
BOOL ShellUi_DrawButton(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, char* Text);
```

boltshell.dll · export `ShellUi_DrawButton1` · SDK 10

As [`CmnCtl_DrawButton`](cmnctl.md#cmnctl_drawbutton).

### ShellUi_DrawCheckbox

```c
BOOL ShellUi_DrawCheckbox(WORD32 X, WORD32 Y, PSTR Title, PBOOL State);
```

boltshell.dll · export `ShellUi_DrawCheckbox1` · SDK 10

As [`CmnCtl_DrawCheckbox`](cmnctl.md#cmnctl_drawcheckbox).

### ShellUi_DrawComboBox

```c
BOOL ShellUi_DrawComboBox(WORD32 X, WORD32 Y, WORD32 W, PSTR Title,
                          WORD32 ElementCnt, PSTR* Elements, PWORD32 Selected,
                          PBOOL IsOpen);
```

boltshell.dll · export `ShellUi_DrawComboBox1` · SDK 10

As [`CmnCtl_DrawComboBox`](cmnctl.md#cmnctl_drawcombobox).

### ShellUi_DrawTextInput

```c
VOID ShellUi_DrawTextInput(WORD32 X, WORD32 Y, WORD32 W, PSTR Text,
                           PBOOL IsOpen, PWORD32 CursorPos);
```

boltshell.dll · export `ShellUi_DrawTextInput1` · SDK 10

As [`CmnCtl_DrawTextInput`](cmnctl.md#cmnctl_drawtextinput).

### ShellUi_DrawTextInputML

```c
VOID ShellUi_DrawTextInputML(WORD32 X, WORD32 Y, WORD32 W, WORD32 Lines,
                             PSTR Text, PWORD32 CursorX, PWORD32 CursorY,
                             PBOOL IsInFocus, PSTR** RichCtx);
```

boltshell.dll · export `ShellUi_DrawTextInputML1` · SDK 10

As [`CmnCtl_DrawTextInputML`](cmnctl.md#cmnctl_drawtextinputml).

### ShellUi_DrawIntInput

```c
BOOL ShellUi_DrawIntInput(WORD32 X, WORD32 Y, WORD32 W, INT Min, INT Max,
                          PINT Value);
```

boltshell.dll · export `ShellUi_DrawIntInput1` · SDK 10

As [`CmnCtl_DrawIntInput`](cmnctl.md#cmnctl_drawintinput).

### ShellUi_DrawDropMenu

```c
VOID ShellUi_DrawDropMenu(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon,
                          PBOOL State, PVOID StateChangedCb);
```

boltshell.dll · export `ShellUi_DrawDropMenu1` · SDK 10

As [`CmnCtl_DrawDropMenu`](cmnctl.md#cmnctl_drawdropmenu).

### ShellUi_DrawDropItem

```c
VOID ShellUi_DrawDropItem(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon);
```

boltshell.dll · export `ShellUi_DrawDropItem1` · SDK 10

As [`CmnCtl_DrawDropItem`](cmnctl.md#cmnctl_drawdropitem).

### ShellUi_DrawDropItemSelectable

```c
BOOL ShellUi_DrawDropItemSelectable(WORD32 X, WORD32 Y, WORD32 W, PSTR Name,
                                    PVOID Icon, BOOL Selected);
```

boltshell.dll · export `ShellUi_DrawDropItemSelectable1` · SDK 10

As [`CmnCtl_DrawDropItemSelectable`](cmnctl.md#cmnctl_drawdropitemselectable).

### ShellUi_CreateWindow

```c
HANDLE ShellUi_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H,
                            char* Title, void* Icon16, WORD16 ObjId,
                            void* Function);
```

boltshell.dll · export `ShellUi_CreateWindow1` · SDK 10

Makes a window handle carrying an immediate-mode procedure, `Function`, the
way windows were made before the window manager owned them. The handle is not
registered with the window manager and nothing draws it unless its maker does,
with [`ShellUi_DrawWindow`](#shellui_drawwindow). Kept for what was written
against it; an application makes windows with
[`Wndmgr_CreateWindow`](wndmgr.md#wndmgr_createwindow).

### ShellUi_DrawWindow

```c
void ShellUi_DrawWindow(HANDLE Window, BOOL WindowHasFocus);
```

boltshell.dll · export `ShellUi_DrawWindow1` · SDK 10

Draws a window made by [`ShellUi_CreateWindow`](#shellui_createwindow) — its
frame, title bar and procedure — at its position, kept on the screen.
