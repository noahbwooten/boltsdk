#pragma once
/*
wndproc2.h
BoltOS Usermode Include Declarations
The message-driven window procedure

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 __WindowProcedure2: what a window is told, rather than what it is asked to
 redraw. The window manager sends a message when something has happened, so a
 window with nothing to do costs nothing. It falls back to __WindowProcedure
 when a module does not export this.

     WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message,
                               WORD64 Param1, WORD64 Param2);

 Geometry comes from the window handle rather than from arguments, so new
 messages never change the signature. Return zero unless a message says
 otherwise.
 */

/* -- Messages -- */

/*
 Numbering starts at one. Zero is deliberately not a message: a zeroed
 structure, a cleared register and an uninitialised pointer all produce zero.
 */

/* The window has just been created. Sent once, before the first paint.
   Allocate here rather than in __modmain: __modmain runs when the module is
   loaded, which is not the same moment, and a module may outlive a window. */
#define MSG_INIT      0x01

/*
 Shut down now. Reserved, not yet sent. When it is, the application will be
 terminated 250 ms later whether it has returned or not. The number is
 allocated now so nothing else can take it.
 */
#define MSG_KILL      0x02

/*
 The close button was pressed. Return zero to allow the close, non-zero to
 refuse it, which is what an application with unsaved work does while it puts a
 question on screen. A refusal should last only as long as that dialogue.
 */
#define MSG_QUIT      0x03

/*
 Minimised. Stop drawing, and give back anything expensive. The window is not
 being destroyed and its state must survive. A backing surface is over a
 megabyte and is worth releasing here; MSG_RESTORE undoes it.
 */
#define MSG_MINIMIZE  0x04

/* Visible again after MSG_MINIMIZE. Reallocate whatever MSG_MINIMIZE released.
   Always followed by a paint, so there is no need to draw from here. */
#define MSG_RESTORE   0x05

/*
 Draw. Param1 carries the damage rectangle, packed as four 16-bit fields:

     x = Param1 & 0xFFFF, y = (Param1 >> 16) & 0xFFFF,
     w = (Param1 >> 32) & 0xFFFF, h = (Param1 >> 48) & 0xFFFF

 Today it is always the whole window. Drawing is in window coordinates: (0, 0)
 is the top-left of the window and stays there when it is dragged. This is the
 only message during which drawing is valid, because the window manager points
 the primitives at the window's surface for its duration only.
 */
#define MSG_PAINT     0x06

/*
 The pointer moved. Param1 packs the position, window-relative:
 x = Param1 & 0xFFFF, y = (Param1 >> 16) & 0xFFFF. Param2 is the button state,
 as a bitmask of (1 << code) using the button codes below.
 */
#define MSG_CURSOR    0x07

/* A key or mouse button went down, or came back up. Param1 is the code. These
   are edges, not levels: one message per transition, however many frames the
   key is held -- except that a key (not a button, and not a modifier) held
   for a second repeats, and each repeat is another MSG_KEYDOWN, followed by
   its MSG_CHAR, with Param2 one. Param2 is zero otherwise. */
#define MSG_KEYDOWN   0x08
#define MSG_KEYUP     0x09

/*
 A character was typed. Param1 is the character itself. Separate from
 MSG_KEYDOWN because a key code is not a character: shift, the layout and the
 repeat rate all sit between the two.
 */
#define MSG_CHAR      0x0A

/* Gained or lost the focus. Previously an application could only infer this
   from being handed a zeroed mouse position, which is indistinguishable from
   the pointer genuinely being at the origin. */
#define MSG_FOCUS     0x0B
#define MSG_BLUR      0x0C

/* A second has passed and this window is visible. Param1 is the current time in
   the same form User_GetKTime returns. Distinct from __modclock's MODMSG_1SEC,
   which is sent to the module rather than to a window and arrives whether the
   window is on screen or not. */
#define MSG_TIMER     0x0D

/* Reserved. Nothing moves a window yet, so this is a number held open rather
   than a message that is sent. */
#define MSG_MOVE      0x0E

/*
 About to be resized to the size in Param1, packed as MSG_MAKEPOINT. Zero
 accepts and non-zero refuses, as MSG_QUIT does, so ignoring this is accepting.
 The size is already clamped, so a handler may lay itself out before returning.
 */
#define MSG_RESIZE    0x0F

/*
 The wheel has turned, and this window has the focus. Param1 is the notch count,
 signed positive away from the user; Param2 is the pointer, window relative and
 packed as MSG_MAKEPOINT. Sent only to the focused window, so an application
 that wants the wheel wherever the pointer is asks for the focus first.
 */
#define MSG_SCROLL    0x10

/*
 A registered interval has come due. Param1 is the identifier Wndmgr_SetTimer
 returned, Param2 the millisecond clock. Sent outside a paint, so invalidate
 rather than draw, and tested once a frame rather than promised.
 */
#define MSG_INTERVAL  0x11

/* -- Button codes -- */

/*
 Mouse buttons live in the key code space, at the bottom of it. BoltOS key
 codes reserve 0x01, 0x02 and 0x04 to 0x06 for the mouse and use none of them
 for keys.
 */
#define MSGKEY_LBUTTON  0x01
#define MSGKEY_RBUTTON  0x02
#define MSGKEY_MBUTTON  0x04
#define MSGKEY_XBUTTON1 0x05
#define MSGKEY_XBUTTON2 0x06

/* -- Packing helpers -- */

#define MSG_MAKEPOINT(x, y)   (((WORD64)((y) & 0xFFFF) << 16) | ((WORD64)((x) & 0xFFFF)))
#define MSG_POINTX(p)         ((WORD32)((p) & 0xFFFF))
#define MSG_POINTY(p)         ((WORD32)(((p) >> 16) & 0xFFFF))

#define MSG_MAKERECT(x, y, w, h) \
	(((WORD64)((h) & 0xFFFF) << 48) | ((WORD64)((w) & 0xFFFF) << 32) | \
	 ((WORD64)((y) & 0xFFFF) << 16) | ((WORD64)((x) & 0xFFFF)))
#define MSG_RECTX(r)          ((WORD32)((r) & 0xFFFF))
#define MSG_RECTY(r)          ((WORD32)(((r) >> 16) & 0xFFFF))
#define MSG_RECTW(r)          ((WORD32)(((r) >> 32) & 0xFFFF))
#define MSG_RECTH(r)          ((WORD32)(((r) >> 48) & 0xFFFF))

/* The name the window manager looks for. A module that exports this is driven
   by messages; one that does not is driven the original way. */
#define WNDPROC2_EXPORT_NAME "__WindowProcedure2"

#endif /* BOLTSDK_APIV_10 */
