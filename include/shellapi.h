#pragma once
/*
shellapi.h
BoltOS Usermode Include Declarations
The Shell's Functions, For Applications

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

/*
 What the shell offers an application: starting programs and opening files,
 message boxes, links, the icons and names files are drawn with, and the
 original immediate-mode controls. Exported by the shell, which is
 boltshell.dll and /sys/core/shell.bxf on the volume, so an application that
 uses any of it links against shell.lib.

 The shell's own workings stay in boltshell\shell.h, which includes this.
 */

#include "umbase.h"
#include "user.h"
#include "umkctrl.h"
#include "applink.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call the
 plain names.
 */
#ifdef BOLTSDK_APIV
#define Shell_IconModuleFor            Shell_IconModuleFor1
#define Shell_IconNameFor              Shell_IconNameFor1
#define Shell_TypeNameFor              Shell_TypeNameFor1
#define Shell_ReadAppLink              Shell_ReadAppLink1
#define Shell_WriteAppLink             Shell_WriteAppLink1
#define Shell_ExecuteEx                Shell_ExecuteEx1
#define Shell_OpenFile                 Shell_OpenFile1
#define Shell_OpenFileWith             Shell_OpenFileWith1
#define Shell_Execute                  Shell_Execute1
#define Shell_CloseWindow              Shell_CloseWindow1
#define Shell_MinimizeWindow           Shell_MinimizeWindow1
#define Shell_GetColors                Shell_GetColors1
#define Shell_GetShellRes              Shell_GetShellRes1
#define Shell_SetClips                 Shell_SetClips1
#define Shell_GetWindowData            Shell_GetWindowData1
#define Shell_MessageBox               Shell_MessageBox1
#define Shell_GetMessageBoxId          Shell_GetMessageBoxId1
#define Shell_GetMessageBoxStatus      Shell_GetMessageBoxStatus1
#define Shell_BringForth               Shell_BringForth1
#define ShellUi_CreateWindow           ShellUi_CreateWindow1
#define ShellUi_DrawWindow             ShellUi_DrawWindow1
#define ShellUi_DrawButton             ShellUi_DrawButton1
#define ShellUi_DrawCheckbox           ShellUi_DrawCheckbox1
#define ShellUi_DrawComboBox           ShellUi_DrawComboBox1
#define ShellUi_DrawTextInput          ShellUi_DrawTextInput1
#define ShellUi_DrawTextInputML        ShellUi_DrawTextInputML1
#define ShellUi_DrawIntInput           ShellUi_DrawIntInput1
#define ShellUi_DrawDropMenu           ShellUi_DrawDropMenu1
#define ShellUi_DrawDropItem           ShellUi_DrawDropItem1
#define ShellUi_DrawDropItemSelectable ShellUi_DrawDropItemSelectable1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 Exported by the shell and imported by everything else. Spelled as an import
 everywhere but the shell, since an application that wrote its own export
 with this would be telling the compiler two things about one symbol.
 */
#ifdef _USERMODE_APP
#define SHELLAPI_EXPORT __declspec(dllimport)
#else
#define SHELLAPI_EXPORT __declspec(dllexport)
#endif

/* The palette the shell paints with. Shell_GetColors hands back the shell's
   own copy, which is read and not written. */
typedef struct _SHELL_COLORS {
	WORD32 TextColor;
	WORD32 TextColorDark;
	WORD32 TextColorGrey;
	WORD32 BackgroundColor;
	WORD32 AccentColor;

	WORD32 TaskbarBackground;
	WORD32 TaskbarOutline;
	WORD32 TaskbarClockBackground;
	WORD32 TaskbarClockOutline;
	WORD32 TaskbarItemInActiveBackground;
	WORD32 TaskbarItemActiveBackground;
	WORD32 TaskbarItemInActiveHover;
	WORD32 TaskbarItemActiveHover;
	WORD32 TaskbarItemOutline;
	WORD32 TaskbarActionMenuActive;
	WORD32 TaskbarActionMenu;
	WORD32 TaskbarActionMenuActiveText;
	WORD32 TaskbarActionMenuOutline;

	WORD32 ActionMenuBackground;
	WORD32 ActionMenuOutline;
	WORD32 ActionMenuBackgroundHover;
	WORD32 ActionMenuBackgroundHeader;

	WORD32 ShellWindowBackground, ShellWindowBackgroundDark;
	WORD32 ShellWindowTitleBarBackground, ShellWindowTitleBarBackgroundDark;
	WORD32 ShellWindowTitleBarBackgroundNonFocus;
	WORD32 ShellWindowOutline, ShellWindowOutlineDark;
	WORD32 ShellWindowCtrls_CloseButton;
	WORD32 ShellWindowCtrls_CloseButtonHover;
	WORD32 ShellWindowCtrls_MinimizeButton;
	WORD32 ShellWindowCtrls_MinimizeButtonHover;

	WORD32 ShellElement_Button_Outline;
	WORD32 ShellElement_Button_Fill;
	WORD32 ShellElement_Button_FillHover;
	WORD32 ShellElement_ComboBox_Item;
	WORD32 ShellElement_ComboBox_ItemHover;
	WORD32 ShellElement_TabBar_Background;
	WORD32 ShellElement_TabBar_Item;
	WORD32 ShellElement_TabBar_ItemHovered;
	WORD32 ShellElement_TabBar_ItemSelect;
	WORD32 ShellElement_TabBar_Inactive;
	WORD32 ShellElement_TabBar_InactiveHover;

	WORD32 PowerOptionsBackground;
}SHELL_COLORS, * PSHELL_COLORS;

/*
 What a window should look like when the application does not say. Handed to
 Shell_ExecuteEx by a launcher that has the information from somewhere else, and
 used only when the entry point opened no windows of its own.
 */
typedef struct _SHELL_LAUNCHINFO {
	WORD32 X, Y, W, H;

	/* Copied by the shell under the application being started, since a window
	   keeps the pointer it is given and a launcher may close before it. */
	char Title[64];

	/* An image by name, as a manifest names one, read out of the module being
	   started and loaded under it for the same reason as the title above.
	   Empty means no icon. See modres.h. */
	char IconName[32];

	WORD32 Flags;
}SHELL_LAUNCHINFO, * PSHELL_LAUNCHINFO;

/* A window's handle slots and flags, as ShellUi_CreateWindow makes one. They
   are the window manager's, WNDMGRHND_TYPE_* and WNDMGRHNDFLAGS_*, under the
   names the shell gave them first. */
#define SHELLHND_TYPE_WINDOWX 0
#define SHELLHND_TYPE_WINDOWY 1
#define SHELLHND_TYPE_WINDOWW 2
#define SHELLHND_TYPE_WINDOWH 3
#define SHELLHND_TYPE_ICON 4
#define SHELLHND_TYPE_TEXT 5
#define SHELLHND_TYPE_OBJID 6
#define SHELLHND_TYPE_FUNCTION 7
#define SHELLHND_TYPE_FLAGS 8
#define SHELLHND_TYPE_MODULE 9
#define SHELLHND_TYPE_NTVMD 10
#define SHELLHND_TYPE_MDID 11
#define SHELLHND_TYPE_ARGUMENT 12
#define SHELLHND_TYPE_ICON32 13

/* The window's backing surface, and the size it was allocated at. The size is
   stored rather than derived from the window so that a resize is detectable:
   the window's dimensions change first, and the mismatch is what triggers the
   reallocation. */
#define SHELLHND_TYPE_SURFACE 14
#define SHELLHND_TYPE_SURFACEW 15
#define SHELLHND_TYPE_SURFACEH 16
#define SHELLHND_TYPE_V2STATE 17
#define SHELLHND_TYPE_FUNCTION2 18
#define SHELLHND_TYPE_CURSOR 24

#define SHELLHNDFLAGS_ISOPEN        0x00000001
#define SHELLHNDFLAGS_NOTASKBARITEM 0x00000002
#define SHELLHNDFLAGS_NOAUTOPAINT   0x00000004
#define SHELLHNDFLAGS_NOICON        0x00000008
#define SHELLHNDFLAGS_PASSARGUMENT  0x00000010
#define SHELLHNDFLAGS_NOMINIMIZE    0x00000020
#define SHELLHNDFLAGS_NOCLOSE       0x00000040
#define SHELLHNDFLAGS_CENTERTEXT    0x00000080
#define SHELLHNDFLAGS_DARKBKGRND    0x00000100

/* This window is driven by __WindowProcedure2. Set at creation by whoever
   found the export, rather than inferred later from the pointer being
   non-null, since a window with no procedure would then look like a v2 one. */
#define SHELLHNDFLAGS_MSGPROC      0x00000400

/* Not resizable. See WNDMGRHNDFLAGS_NORESIZE, which this mirrors. */
#define SHELLHNDFLAGS_NORESIZE     0x00000800

/* The two shift keys, as the immediate-mode text inputs read them. */
#define KS_LSHIFT 16
#define KS_RSHIFT 116

/* -- Message boxes -- */

#define SHELL_MSGBOXICON_NONE 0
#define SHELL_MSGBOXICON_INFO 1
#define SHELL_MSGBOXICON_WARN 2
#define SHELL_MSGBOXICON_EROR 3
#define SHELL_MSGBOXICON_CTCL 4
#define SHELL_MSGBOXICON_QSTN 5

#define SHELL_MSGBOXBTN_OK       0x0001
#define SHELL_MSGBOXBTN_CANCEL   0x0002
#define SHELL_MSGBOXBTN_RETRY    0x0004
#define SHELL_MSGBOXBTN_ABORT    0x0008
#define SHELL_MSGBOXBTN_FAIL     0x0010
#define SHELL_MSGBOXBTN_SAVE     0x0020
#define SHELL_MSGBOXBTN_DONTSAVE 0x0040
#define SHELL_MSGBOXBTN_YES      0x0080
#define SHELL_MSGBOXBTN_NO       0x0100

/*
 The box makes no sound of its own, because the caller is making one. For an
 alert that is already sounding, such as an alarm: without this the box's own
 chime lands on top of it at the moment it goes off.

 Well clear of the buttons, which occupy the low nine bits and are passed
 through to the system modal as they are.
 */
#define SHELL_MSGBOXOPT_SILENT   0x8000

#define SHELL_MSGBOXRSLT_WAIT 0
#define SHELL_MSGBOXRSLT_OK 1
#define SHELL_MSGBOXRSLT_CANCEL 2
#define SHELL_MSGBOXRSLT_RETRY 3
#define SHELL_MSGBOXRSLT_ABORT 4
#define SHELL_MSGBOXRSLT_FAIL 5
#define SHELL_MSGBOXRSLT_SAVE 6
#define SHELL_MSGBOXRSLT_DONTSAVE 7
#define SHELL_MSGBOXRSLT_YES 8
#define SHELL_MSGBOXRSLT_NO 9

/* -- Where things are -- */

/*
 Where the system's applications are, and what one is called. Nothing scans
 this folder any more: what the Actions menu offers is APPLINK_ACTIONS_PATH,
 and an application is reachable from here by a link somebody put there.
 */
#define SHELL_APPS_PATH   "/sys/apps/"
#define SHELL_APPS_SUFFIX ".bxf"

/*
 The two archives, named where both the icon a name earns and the thing that
 opens one can see them. The extensions are what the rest of the world uses,
 which is the whole point of them: an archive written here opens elsewhere and
 one written elsewhere opens here.
 */
#define SHELL_ARCHIVE_ZIP ".zip"
#define SHELL_ARCHIVE_TAR ".tar"

/* An application package, which is opened rather than run: what a double
   click on one has to start is the installer, not the application inside it.
   The extension itself is BXI_SUFFIX, in bxi.h with the rest of the format. */
#define SHELL_PACKAGE_APP "/sys/utils/bxiutil.bxf"

/* -- What a file is drawn as -- */

/*
 The icon a path earns, by name. Empty applies to a directory and
 decides between the two folder icons; a link answers with the icon it names,
 or with the icon of what it points at when it names none.

 FALSE means nothing was decided and the caller draws no icon.
 */
/* The module carrying the icon that name is in, for a link, which points at an
   application and names an icon inside it. FALSE means the shell's own. */
SHELLAPI_EXPORT BOOL Shell_IconModuleFor(PSTR Path, PSTR Out, WORD32 Size);

SHELLAPI_EXPORT BOOL Shell_IconNameFor(PSTR Path, WORD32 Type, BOOL Empty, PSTR Out,
	WORD32 Size);

/* What a path is called, as a reader would say it: "Folder", "Text Document
   (.txt)", or "TMP File (.tmp)" for an extension nothing here knows. Type is a
   USER_FILETYPE_*. */
SHELLAPI_EXPORT BOOL Shell_TypeNameFor(PSTR Path, WORD32 Type, PSTR Out,
	WORD32 Size);

/* -- Links -- */

/* Read one link, and write one. A link that is short, or that carries a magic
   or a version this build does not know, reads as absent. */
SHELLAPI_EXPORT BOOL Shell_ReadAppLink(PSTR Path, PAPPLINK Out);
SHELLAPI_EXPORT BOOL Shell_WriteAppLink(PSTR Path, PAPPLINK Link);

/* -- Starting things -- */

/*
 Load an application and run its entry point, which is where it creates its
 windows. Opt_Info is for a launcher that knows how a window should look for an
 application that does not open one itself; null means the application decides,
 and an application that opens nothing is simply left running.

 Null when it could not be started. One built against a newer Bolt SDK than
 the system provides is refused, and the shell says so.
 */
SHELLAPI_EXPORT PVOID Shell_ExecuteEx(PSTR FilePath, PSHELL_LAUNCHINFO Opt_Info,
	PWORD16 Opt_OutModuleId);

/*
 Open a file by what its name ends in: an application is started, and a
 document is handed to the application that reads it. Returns false when
 nothing is associated with the name, which is the caller's to report.
 */
SHELLAPI_EXPORT BOOL Shell_OpenFile(PSTR Path);

/*
 Start an application and hand it a document, for a caller that has decided
 which application itself. The document reaches it through MODOPEN_EXPORT_NAME,
 so one that does not export that is started with nothing open.
 */
SHELLAPI_EXPORT BOOL Shell_OpenFileWith(PSTR AppPath, PSTR DocumentPath);

/* The older form of Shell_ExecuteEx, kept for what still calls it. ObjId is
   not used, and NativeModule is always given back null. */
SHELLAPI_EXPORT PVOID Shell_Execute(char* FilePath, WORD16 ObjId, PVOID* NativeModule, WORD16* ModId);

/* -- Windows -- */

SHELLAPI_EXPORT void Shell_CloseWindow(HANDLE Window);
SHELLAPI_EXPORT void Shell_MinimizeWindow(HANDLE Window);
SHELLAPI_EXPORT PSHELL_COLORS Shell_GetColors(VOID);
SHELLAPI_EXPORT PKMCTL_SHELLRES Shell_GetShellRes(VOID);
SHELLAPI_EXPORT VOID Shell_SetClips(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
SHELLAPI_EXPORT VOID Shell_GetWindowData(PWORD16 MouseX, PWORD16 MouseY, PBOOL LeftButton);
SHELLAPI_EXPORT VOID   Shell_BringForth(HANDLE Window);

/*
 A message box, which is a window of its own and does not wait: the call
 returns an id at once, and the answer is asked for later with
 Shell_GetMessageBoxStatus, which says SHELL_MSGBOXRSLT_WAIT until somebody
 presses something. Image is a SHELL_MSGBOXICON_* and Options the
 SHELL_MSGBOXBTN_* buttons, with SHELL_MSGBOXOPT_SILENT if wanted.
 */
SHELLAPI_EXPORT WORD32 Shell_MessageBox(PSTR Title, PSTR Message, BYTE Image, WORD16 Options);
SHELLAPI_EXPORT WORD32 Shell_GetMessageBoxId(PSTR Title, PSTR Message);
SHELLAPI_EXPORT WORD32 Shell_GetMessageBoxStatus(WORD32 Id);

/* -- The immediate-mode controls -- */

/*
 The original controls, drawn and answered in one call every frame, as cmnctl.h
 has them too. The retained controls in cmnctl2.h are what a new application
 should use; these are here for the ones written against them.
 */
SHELLAPI_EXPORT HANDLE ShellUi_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, char* Title, void* Icon16, WORD16 ObjId, void* Function);
SHELLAPI_EXPORT void ShellUi_DrawWindow(HANDLE Window, BOOL WindowHasFocus);
SHELLAPI_EXPORT BOOL ShellUi_DrawButton(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, char* Text);
SHELLAPI_EXPORT BOOL ShellUi_DrawCheckbox(WORD32 X, WORD32 Y, PSTR Title, PBOOL State);
SHELLAPI_EXPORT BOOL ShellUi_DrawComboBox(WORD32 X, WORD32 Y, WORD32 W, PSTR Title, WORD32 ElementCnt, PSTR* Elements, PWORD32 Selected, PBOOL IsOpen);
SHELLAPI_EXPORT VOID ShellUi_DrawTextInput(WORD32 X, WORD32 Y, WORD32 W, PSTR Text, PBOOL IsOpen, PWORD32 CursorPos);
SHELLAPI_EXPORT VOID ShellUi_DrawTextInputML(WORD32 X, WORD32 Y, WORD32 W, WORD32 Lines, PSTR Text, PWORD32 CursorX, PWORD32 CursorY, PBOOL IsInFocus, PSTR** RichCtx);
SHELLAPI_EXPORT BOOL ShellUi_DrawIntInput(WORD32 X, WORD32 Y, WORD32 W, INT Min, INT Max, PINT Value);
SHELLAPI_EXPORT VOID ShellUi_DrawDropMenu(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon, PBOOL State, PVOID StateChangedCb);
SHELLAPI_EXPORT VOID ShellUi_DrawDropItem(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon);
SHELLAPI_EXPORT BOOL ShellUi_DrawDropItemSelectable(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon, BOOL Selected);

#endif /* BOLTSDK_APIV_10 */
