#pragma once
#include "umbase.h"
#include "umkctrl.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define Wndmgr_SetContentHeight     Wndmgr_SetContentHeight1
#define Wndmgr_GetScrollY           Wndmgr_GetScrollY1
#define Wndmgr_VisibleWidth         Wndmgr_VisibleWidth1
#define Wndmgr_GetColors            Wndmgr_GetColors1
#define Wndmgr_ScrollMax            Wndmgr_ScrollMax1
#define Wndmgr_ScrollNeeded         Wndmgr_ScrollNeeded1
#define Wndmgr_ScrollClamp          Wndmgr_ScrollClamp1
#define Wndmgr_PaintScrollBar       Wndmgr_PaintScrollBar1
#define Wndmgr_ScrollPart           Wndmgr_ScrollPart1
#define Wndmgr_ScrollStep           Wndmgr_ScrollStep1
#define Wndmgr_ScrollFromPoint      Wndmgr_ScrollFromPoint1
#define Wndmgr_ScrollGrab           Wndmgr_ScrollGrab1
#define Wndmgr_CreateWindow         Wndmgr_CreateWindow1
#define Wndmgr_LoadIconFrom         Wndmgr_LoadIconFrom1
#define Wndmgr_LoadIcon             Wndmgr_LoadIcon1
#define Wndmgr_SetDesktopPainter    Wndmgr_SetDesktopPainter1
#define Wndmgr_WindowAtPoint        Wndmgr_WindowAtPoint1
#define Wndmgr_ListWindowAtPoint    Wndmgr_ListWindowAtPoint1
#define Wndmgr_SetOverlay           Wndmgr_SetOverlay1
#define Wndmgr_KeysOnDesktop        Wndmgr_KeysOnDesktop1
#define Wndmgr_ModuleHasWindows     Wndmgr_ModuleHasWindows1
#define Wndmgr_ModuleWindowCount    Wndmgr_ModuleWindowCount1
#define Wndmgr_SetTimer             Wndmgr_SetTimer1
#define Wndmgr_KillTimer            Wndmgr_KillTimer1
#define Wndmgr_CloseWindow          Wndmgr_CloseWindow1
#define Wndmgr_CloseModuleWindows   Wndmgr_CloseModuleWindows1
#define Wndmgr_MinimizeWindow       Wndmgr_MinimizeWindow1
#define Wndmgr_FocusVisible         Wndmgr_FocusVisible1
#define Wndmgr_GetShellRes          Wndmgr_GetShellRes1
#define Wndmgr_SetClips             Wndmgr_SetClips1
#define Wndmgr_GetWindowData        Wndmgr_GetWindowData1
#define Wndmgr_BringForth           Wndmgr_BringForth1
#define Wndmgr_RequestSpecialObject Wndmgr_RequestSpecialObject1
#define Wndmgr_DrawWindow           Wndmgr_DrawWindow1
#define Wndmgr_InvalidateWindow     Wndmgr_InvalidateWindow1
#define Wndmgr_SetWindowCursor      Wndmgr_SetWindowCursor1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define WNDMGRHND_TYPE_WINDOWX 0
#define WNDMGRHND_TYPE_WINDOWY 1
#define WNDMGRHND_TYPE_WINDOWW 2
#define WNDMGRHND_TYPE_WINDOWH 3

/* The title bar, which is where a window's client area starts. The frame
   drawing works in it throughout, and anything that measures against the
   client — the controls' open lists among them — names it rather than
   writing 30 of its own. */
#define WNDMGR_TITLEH 30
#define WNDMGRHND_TYPE_ICON 4
#define WNDMGRHND_TYPE_TEXT 5
#define WNDMGRHND_TYPE_OBJID 6
#define WNDMGRHND_TYPE_FUNCTION 7
#define WNDMGRHND_TYPE_FLAGS 8
#define WNDMGRHND_TYPE_MODULE 9
#define WNDMGRHND_TYPE_NTVMD 10
#define WNDMGRHND_TYPE_MDID 11
#define WNDMGRHND_TYPE_ARGUMENT 12
#define WNDMGRHND_TYPE_ICON32 13

/* The window's backing surface, and the size it was allocated at. The size is
   stored rather than derived from the window so that a resize is detectable:
   the window's dimensions change first, and the mismatch is what triggers the
   reallocation. */
#define WNDMGRHND_TYPE_SURFACE 14
#define WNDMGRHND_TYPE_SURFACEW 15
#define WNDMGRHND_TYPE_SURFACEH 16

/* Everything the message-driven procedure needs to remember between frames,
   as bits in one slot rather than one slot each. Handle slots are a fixed
   resource and a window has no business spending four of them on booleans. */
#define WNDMGRHND_TYPE_V2STATE 17
#define WNDMGRHND_TYPE_FUNCTION2 18

/*
 Where an open combo box's list hangs, packed as four sixteen bit fields —
 x, y, width and height in the window's own coordinates, the same space the
 application draws in. Written by the controls while a list is down and
 cleared when none is; the window manager reads it to paint the part of the
 list past the window's edge and to leave a click on a visible row with the
 window that put it there. Zero when no list is open.

 Slot 21, past the controls' own two: a control window keeps its list of
 controls in the handle too, at 19 and 20, and a rect written over the head
 of that list is every control on the window gone.
 */
#define WNDMGRHND_TYPE_LISTRECT 21

/*
 The height the window's content wants, and how far down it the client is
 looking. Zero content means a window that does not scroll, which is every
 window that has not asked to.
 */
#define WNDMGRHND_TYPE_CONTENTH 22
#define WNDMGRHND_TYPE_SCROLLY  23

/*
 What the pointer should look like over this window's client area. Stored, not
 hinted: a hint is cleared each frame and a retained control does not re-assert
 it. Stored as the cursor type plus one, so zero means no preference.

 Slot 24. It shared 21 with the list rectangle above until SDK 10 was made,
 so a window with a list open and a pointer of its own had each written over
 the other. Set it with Wndmgr_SetWindowCursor rather than through the slot.
 */
#define WNDMGRHND_TYPE_CURSOR 24

#define WNDV2_INITIALISED 0x00000001  /* MSG_INIT has been sent */
#define WNDV2_HADFOCUS    0x00000002  /* had the focus last frame */
#define WNDV2_DIRTY       0x00000004  /* needs repainting */
#define WNDV2_MINIMISED   0x00000008  /* MSG_MINIMIZE sent, MSG_RESTORE owed */

#define WNDMGRHNDFLAGS_ISOPEN        0x00000001
#define WNDMGRHNDFLAGS_NOTASKBARITEM 0x00000002
#define WNDMGRHNDFLAGS_NOAUTOPAINT   0x00000004
#define WNDMGRHNDFLAGS_NOICON        0x00000008
#define WNDMGRHNDFLAGS_PASSARGUMENT  0x00000010
#define WNDMGRHNDFLAGS_NOMINIMIZE    0x00000020
#define WNDMGRHNDFLAGS_NOCLOSE       0x00000040
#define WNDMGRHNDFLAGS_CENTERTEXT    0x00000080
#define WNDMGRHNDFLAGS_DARKBKGRND    0x00000100
#define WNDMGRHNDFLAGS_RAWWNDPROC    0x00000200

/* This window is driven by __WindowProcedure2. Set at creation by whoever
   found the export, rather than inferred later from the pointer being
   non-null, since a window with no procedure would then look like a v2 one. */
#define WNDMGRHNDFLAGS_MSGPROC      0x00000400

/* No edge to resize it by. For a window whose contents cannot be laid out at
   another size, which is every window that does not answer MSG_RESIZE, since
   ignoring that message accepts the resize. */
#define WNDMGRHNDFLAGS_NORESIZE     0x00000800

/* Drawn over the other windows whatever the focus is doing, short of the
   focused window itself, which still lands over these. For a gadget pinned
   over the work rather than buried under it. */
#define WNDMGRHNDFLAGS_TOPMOST      0x00001000

/* Painting is not clipped to the window while this stands, because one of
   its controls has a list hanging open past the edge: the open part is
   painted every frame from the window's own surface, which is grown to hold
   it, and the band below the window is blitted beside the window itself.
   Set and cleared by the controls when a list opens and closes; the window
   manager only ever asks whether it stands. */
#define WNDMGRHNDFLAGS_LISTOVERHANG 0x00002000

#define UMI_EXPORT __declspec(dllexport)

typedef struct _WNDMGR_COLORS {
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
}WNDMGR_COLORS, * PWNDMGR_COLORS;

/*
 Declare a client area taller than the window itself.

 The window then grows a bar down its right edge, scrolls under the wheel, and
 hands the application paint and pointer messages in content coordinates: an
 application draws its whole layout at every size and lets the window manager
 decide what is on screen. That is what lets a window be resized smaller than
 its content needs, so an application that declares a height can accept every
 MSG_RESIZE rather than refusing the ones it cannot lay out for.

 Zero turns it off. Horizontal is not offered: nothing in the tree lays out
 sideways, and a second bar wants a corner between the two.
 */
UMI_EXPORT VOID Wndmgr_SetContentHeight(HANDLE Window, WORD32 Height);
UMI_EXPORT WORD32 Wndmgr_GetScrollY(HANDLE Window);

/* How far across a window its client can be seen: the whole width, or up to
   where that bar starts while the window is showing it. Window relative, for a
   control laying itself out that would otherwise put something under the bar. */
UMI_EXPORT WORD32 Wndmgr_VisibleWidth(HANDLE Window);

UMI_EXPORT PWNDMGR_COLORS Wndmgr_GetColors(void);

/* -- Scrollbars -- */

/*
 One width for every bar in the system, so a window, a table and a tree all
 leave the same strip and a pointer crossing between them finds the same target.
 The thumb never shrinks below a size that can still be grabbed.
 */
#define WNDMGR_SCROLL_W         14
#define WNDMGR_SCROLL_MINTHUMB  16
#define WNDMGR_SCROLL_ARROW      4

#define WNDMGR_SCROLLPART_NONE     0
#define WNDMGR_SCROLLPART_UP       1
#define WNDMGR_SCROLLPART_PAGEUP   2
#define WNDMGR_SCROLLPART_THUMB    3
#define WNDMGR_SCROLLPART_PAGEDOWN 4
#define WNDMGR_SCROLLPART_DOWN     5

/*
 The geometry and the one painter, shared rather than copied: cmnctl2 draws the
 bar a control carries and this module draws the bar a window carries, and a
 pointer moving between the two meets the same target in the same place.
 */
UMI_EXPORT WORD32 Wndmgr_ScrollMax(WORD32 Total, WORD32 Visible);
UMI_EXPORT BOOL Wndmgr_ScrollNeeded(WORD32 Total, WORD32 Visible);
UMI_EXPORT WORD32 Wndmgr_ScrollClamp(WORD32 Total, WORD32 Visible, int Position);

UMI_EXPORT VOID Wndmgr_PaintScrollBar(WORD32 X, WORD32 Y, WORD32 H, WORD32 Total,
	WORD32 Visible, WORD32 Position, BOOL Disabled);

UMI_EXPORT WORD32 Wndmgr_ScrollPart(WORD32 Y, WORD32 H, WORD32 Total,
	WORD32 Visible, WORD32 Position, WORD32 PointY);
UMI_EXPORT WORD32 Wndmgr_ScrollStep(WORD32 Part, WORD32 Total, WORD32 Visible,
	WORD32 Position);
UMI_EXPORT WORD32 Wndmgr_ScrollFromPoint(WORD32 Y, WORD32 H, WORD32 Total,
	WORD32 Visible, WORD32 Grab, WORD32 PointY);
UMI_EXPORT WORD32 Wndmgr_ScrollGrab(WORD32 Y, WORD32 H, WORD32 Total,
	WORD32 Visible, WORD32 Position, WORD32 PointY);

/*
 A window owned by the module that is running, which an application creates
 from its entry point. Size, position, title and icon are the application's to
 choose, and it may open as many windows as it needs. The module is taken down
 when its last window closes, not when any one of them does.
 */
UMI_EXPORT HANDLE Wndmgr_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H,
	PSTR Title, PVOID Icon, WORD32 Flags);

/*
 An icon a module carries, at the size asked for. Square sizes only.

 Opt_ModulePath names the module to read it out of. NULL means the module
 asking, and a name it does not carry is looked for in the shell, which is
 where what more than one module draws is kept. A path is how the shell draws
 an application's icon without starting the application. See modres.h.
 */
UMI_EXPORT PVOID Wndmgr_LoadIconFrom(PSTR Opt_ModulePath, PSTR ResourceName,
	WORD32 Size);

/* The same, out of whichever module is asking. Named by a manifest. */
UMI_EXPORT PVOID Wndmgr_LoadIcon(PSTR ResourceName, WORD32 Size);

/* -- The desktop -- */

/*
 What paints the background, called once a frame between the backdrop and the
 first window, as void Painter(WORD32 Width, WORD32 Height).

 Registered here rather than drawn by whoever owns it, because the window
 manager is the only module that runs before the windows do. One painter: there
 is one desktop.
 */
UMI_EXPORT VOID Wndmgr_SetDesktopPainter(PVOID Painter);

/*
 The window covering a point, or HANDLE_INVALID for none. For a painter drawing
 behind the windows, which has to know whether a click on the background is one
 or a click on something in front of it.
 */
UMI_EXPORT HANDLE Wndmgr_WindowAtPoint(WORD32 X, WORD32 Y);

/* The window whose open combo list a point lands on, or HANDLE_INVALID for
   none. A row showing past its window's edge belongs to the window that put
   the list there, and this is how the window manager asks. */
UMI_EXPORT HANDLE Wndmgr_ListWindowAtPoint(WORD32 X, WORD32 Y);

/*
 What the shell draws over the windows -- the taskbar, the Actions menu and its
 panels -- as screen rectangles, four WORD16s each (x, y, width, height), up to
 WNDMGR_OVERLAY_MAX of them. A press that lands on one is the shell's: no window
 is raised by it, dragged by it, or handed it, until that button is let go.
 Called every frame with what was drawn; a count of zero clears them.
 */
#define WNDMGR_OVERLAY_MAX 8
UMI_EXPORT VOID Wndmgr_SetOverlay(WORD32 Count, const WORD16* Rects);

/*
 Whether the keyboard is the desktop's. A press on empty desktop takes it from
 the window that had the focus, which stays on top but is drawn as not in use
 and sent no keys; a press on any window, or Wndmgr_BringForth, gives it back.
 */
UMI_EXPORT BOOL Wndmgr_KeysOnDesktop(VOID);

/* Does this module still have a window open. What tells a window closing from
   an application closing. */
UMI_EXPORT BOOL Wndmgr_ModuleHasWindows(WORD32 ModuleId);

/* How many, for a launcher deciding whether an entry point opened anything.
   The count rather than the answer, since a module already showing a window
   can be started again and the question is what this run added. */
UMI_EXPORT WORD32 Wndmgr_ModuleWindowCount(WORD32 ModuleId);

/* How many timers one window may hold at once. The shared pool behind them
   holds sixty four, so four windows can hold their full sixteen. */
#define WNDMGR_MAX_TIMERS 16

/*
 Ask for MSG_INTERVAL every IntervalMs milliseconds. Returns the identifier
 that tells this timer from the window's others, carried in Param1 of every
 message it produces, or zero when there is no room. Released with the window.
 */
UMI_EXPORT WORD32 Wndmgr_SetTimer(HANDLE Window, WORD32 IntervalMs);

/* Stop a timer before its window closes. The window is named as well as the
   identifier, so one window cannot cancel another's timer by guessing. */
UMI_EXPORT void Wndmgr_KillTimer(HANDLE Window, WORD32 Id);

UMI_EXPORT void Wndmgr_CloseWindow(HANDLE Window);

/*
 Close every window a module holds, and leave the module alone. What a killer
 calls first: ending a module frees its heap, and a window that outlives it is
 a taskbar entry and a procedure pointer reading out of freed blocks.
 */
UMI_EXPORT void Wndmgr_CloseModuleWindows(WORD32 ModuleId);
UMI_EXPORT void Wndmgr_MinimizeWindow(HANDLE Window);

/* Move the focus to a window that is drawn on its own. Registering a window
   focuses it, so this is what a caller uses after making one that is not meant
   to be looked at. */
UMI_EXPORT void Wndmgr_FocusVisible(void);
UMI_EXPORT PKMCTL_SHELLRES Wndmgr_GetShellRes(VOID);
UMI_EXPORT VOID Wndmgr_SetClips(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
UMI_EXPORT VOID Wndmgr_GetWindowData(PWORD16 MouseX, PWORD16 MouseY, PBOOL LeftButton);
UMI_EXPORT VOID Wndmgr_BringForth(HANDLE Window);
UMI_EXPORT PVOID Wndmgr_RequestSpecialObject(VOID);

UMI_EXPORT void Wndmgr_DrawWindow(HANDLE Window, BOOL WindowHasFocus);


/* Mark a window as needing to be repainted. The mechanism a retained control
   uses to say that what it draws has changed, so that the window manager does
   not have to repaint everything constantly on the chance that it did. */
UMI_EXPORT void Wndmgr_InvalidateWindow(HANDLE Window);

/* What the pointer looks like over this window's client area. Set it when it
   changes rather than every frame, since the window manager re-applies it.
   Pass CURSOR_POINT to go back to the arrow. */
UMI_EXPORT void Wndmgr_SetWindowCursor(HANDLE Window, WORD32 Cursor);

#endif /* BOLTSDK_APIV_10 */
