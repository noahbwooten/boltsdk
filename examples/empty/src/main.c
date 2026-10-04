/*
 main.c
 An empty Bolt application.

 The least an application needs to run, and nothing else: it opens one window
 and draws one line in it. Copy this folder to start an application of your
 own, change [application] in app.ini, and build it.

 The system finds an application's functions by name, in the file's export
 table, so each one below is exported and spelled exactly as it is here. Which
 of them it must have:

     __modmain            required  called once, when the application starts
     __modres             required  what the file says about itself
     __WindowProcedure2   required  for a window: what happens in it
     __modhalt            optional  called once, when it is closed
     __modclock           optional  called on the system's clock
     __modopen            optional  a document it was asked to open

 None of these are part of the Bolt API, so none of them is numbered or
 versioned: they are the application's own, and the system calls them.
*/

#include <user.h>
#include <graphics.h>
#include <wndmgr.h>
#include <modver.h>

#define __EXPORT __declspec(dllexport)

/* -- What the file says about itself -- */

/*
 The application's name and its icon, by the name of a picture it carries.
 Both come from app.ini through boltapp.h, which the build writes; this one
 carries no pictures, so its icon is empty and its windows get the system's
 placeholder.
 */
static RSRCTYPE_MANIFEST Manifest = {
	BOLTAPP_NAME, BOLTAPP_ICON, 0
};

/*
 The version manifest, in a section of its own so the file explorer can read
 it without starting anything. The build refuses a file without one, and one
 whose version or file name is not what app.ini says.
 */
MODVERSION_PLACE static const RSRCTYPE_VERSION Version = {
	MODVERSION_HEADER,
	BOLTAPP_DESCRIPTION,
	MODVERSION_PRODUCT,
	MODVERSION_COPYRIGHT,
	MODVERSION_LANGUAGE,
	BOLTAPP_FILENAME
};

/*
 Required. The system asks how many resources there are, then each one's type
 and data. Returning the version manifest from here is also what keeps the
 linker from throwing it away as unused.
 */
__EXPORT WORDPTR __modres(unsigned long Index, unsigned long Query) {
	if (Query == MODRES_QUERY_COUNT)
		return 2;

	if (Index == 0 && Query == MODRES_QUERY_TYPE)
		return MODRES_RSRCTYPE_MANIFEST;
	if (Index == 0 && Query == MODRES_QUERY_DATA)
		return (WORDPTR)&Manifest;

	if (Index == 1 && Query == MODRES_QUERY_TYPE)
		return MODRES_RSRCTYPE_VERSION;
	if (Index == 1 && Query == MODRES_QUERY_DATA)
		return (WORDPTR)&Version;

	return 0;
}

/* -- Starting and stopping -- */

/*
 Required. Called once, when the application is started. Open windows here;
 the application keeps running for as long as it has one, and returning from
 here does not end it. The argument is reserved.
 */
__EXPORT int __modmain(void* Reserved) {
	Wndmgr_CreateWindow(120, 120, 360, 160, Manifest.Name,
		Wndmgr_LoadIcon(Manifest.IconName, 24), 0);
	return 0;
}

/*
 Optional. Called once, when the last window has closed and the application
 is being taken down. Give back what __modmain or MSG_INIT took.
 */
__EXPORT void __modhalt(void) {
}

/*
 Optional, and left out here: an application that exports neither of these is
 started without them.

 __modclock is called on the system's clock with a MODMSG_* value, MODMSG_1SEC
 once a second whether a window is showing or not:

     __EXPORT void __modclock(WORD32 Message) { }

 __modopen is called after __modmain with the path of a document the
 application was asked to open, from the file explorer or from
 Shell_OpenFileWith:

     __EXPORT void __modopen(PSTR Path) { }
 */

/* -- The window -- */

/*
 Required, for a window. Everything that happens to a window arrives here as a
 message; see wndproc2.h for all of them. Return zero unless a message says
 otherwise. Drawing is only valid during MSG_PAINT, in window coordinates, and
 below the title bar, which is WNDMGR_TITLEH tall.
 */
__EXPORT WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message,
	WORD64 Param1, WORD64 Param2
) {
	switch (Message) {
	case MSG_INIT:
		/* The window exists. Allocate anything it needs here. */
		return 0;

	case MSG_PAINT:
		Ge_DrawText(16, WNDMGR_TITLEH + 16,
			"An empty Bolt application.",
			Wndmgr_GetColors()->TextColor, GEFONT_NORMAL);
		return 0;

	case MSG_QUIT:
		/* The close button. Zero lets the window close. */
		return 0;
	}

	return 0;
}
