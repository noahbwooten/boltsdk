#pragma once
/*
applink.h
BoltOS Usermode Include Declarations
Application Link File Format

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"
#include "folders.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/* -- The file -- */

/*
 What sits in a .bal file, whole. The version is checked on load and a link
 that does not match is treated as one that is not there, since a layout this
 build does not know reads as fields it never wrote.
 */
#define APPLINK_MAGIC   0x4B4C4142  /* 'BALK' */
#define APPLINK_VERSION 1

#define APPLINK_SUFFIX ".bal"

/* Where the Actions menu reads its list from. A directory inside it is a
   submenu, and the links in that directory are its rows.

   Under the profile rather than under /sys, since what the menu offers is the
   person's list and not the machine's. */
#define APPLINK_ACTIONS_PATH FOLDER_ACTIONS "/"

/*
 One link. Fixed fields rather than a heap of strings, so the file is its own
 record: a reader takes the whole thing in one call and every field is where
 the header says it is whatever the lengths turn out to be.
 */
typedef struct _APPLINK {
	WORD32 Magic;
	WORD32 Version;

	/* What the link is called where it is shown, which is not the title of any
	   window the target opens. */
	char Name[64];

	/* What is opened. An application is run; anything else is handed to
	   whatever reads it, exactly as a path from anywhere else would be. */
	char Path[128];

	/* An image by name, as an application's manifest names one. It is read
	   out of the module Path names, since that is what carries it.
	   Empty means the link is drawn without an icon. */
	char IconName[32];

	/* Reserved. Nothing below __modmain(void*) can carry an argument yet, so
	   this is written and kept and read by nothing. */
	char Arguments[128];

	/*
	 What a window should look like for a target that opens none of its own,
	 which is what the legacy applications are. Ignored by one that opens its
	 own, and ignored entirely when W or H is zero.
	 */
	WORD32 X, Y, W, H;
	WORD32 WindowFlags;

	/* Reserved, both of them. No bit is defined and none is read. */
	WORD32 Flags;
	WORD32 Reserved[8];
}APPLINK, * PAPPLINK;

#endif /* BOLTSDK_APIV_10 */
