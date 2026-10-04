#pragma once
/*
umkctrl.h
BoltOS Usermode Include Declarations
User-Mode Kernel Access / Controller Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/* CURSOR_POINT and friends, shared with the kernel side. Public, since
   Wndmgr_SetWindowCursor takes one. */
#include "cursor.h"

/*
 The shell's resources as the kernel keeps them, which Wndmgr_GetShellRes and
 Shell_GetShellRes hand back. Read, not written, by anything but the shell.
 */
typedef struct _KMCTL_SHELLRES {
	/*
	 The list of installed applications used to sit here, mirrored from the
	 kernel every frame. The shell builds it from the applications' own
	 manifests instead. See SHELL_APP in boltshell\shell.h.
	 */
	/* The cursors, which the kernel draws before any module is loaded and
	   so cannot ask one for. Everything else a module carries in its own
	   file; see modres.h. */
	void* CursorBundle;
	unsigned long CursorBundleSize;
	unsigned long ShellAccentColor;

	unsigned char ClipEnabled;
	unsigned long ClipX, ClipY, ClipW, ClipH;

	/* Field for field with KmCore->ShellResources from here down. This half
	   of the structure is where the two most recently drifted apart, and
	   there is a size check in guiboot.c that now catches it. */
	unsigned char CursorHint;
	unsigned char CursorDepth;
	struct {
		unsigned char Type;
		int Module;
	}CursorStack[CURSOR_STACK_MAX];

	unsigned char IsDraggingWindow;
	short ox, oy;

	void* Cursor, * Background;
	void* Cursors[CURSOR_COUNT];
	unsigned long BackgroundW, BackgroundH;
}KMCTL_SHELLRES, * PKMCTL_SHELLRES;


#endif /* BOLTSDK_APIV_10 */
