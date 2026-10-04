#pragma once
/*
 cursor.h
 (c)Noah Wooten 2023 - 2026, All Rights Reserved

 What the pointer can look like, shared verbatim between the kernel and user
 mode. The order matters: these index KmCore->ShellResources.Cursors directly,
 so add to the end and move CURSOR_COUNT with it.
 */

#define CURSOR_POINT  0   /* the arrow: the default, and what "inherit" resolves to */
#define CURSOR_ACTION 1   /* the hand: something here can be clicked */
#define CURSOR_TEXT   2   /* the I-beam: something here can be typed into */
#define CURSOR_WAIT   3   /* the hourglass: the machine is busy */

/*
 A window's edges, shown by the window manager wherever dragging would resize
 it. Up and down for the top and the bottom, across for the sides, and the two
 diagonals for the corners. The second diagonal has no picture of its own: it is
 the first one mirrored, made when the cursors are loaded.
 */
#define CURSOR_RESIZEV    4   /* the top and the bottom edges */
#define CURSOR_RESIZEH    5   /* the left and the right edges */
#define CURSOR_RESIZENWSE 6   /* the top left and the bottom right corners */
#define CURSOR_RESIZENESW 7   /* the top right and the bottom left corners */

#define CURSOR_COUNT  8

/*
 How deep an application may push before it is refused. Eight is far more than
 any sane use, so the depth is really a leak detector: anything approaching it
 means pushes are being made without matching pops.
 */
#define CURSOR_STACK_MAX 8
