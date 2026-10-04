/*
 main.c
 (c)Noah Wooten 2023 - 2026, All Rights Reserved

 What the system calls in the calculator: the entry point, the exit, and the
 description of the module the system reads without running anything. The
 calculator itself is in calc.c.

 Everything the file says about itself -- its name, its icon, its version, the
 name it was built under -- comes from app.ini, through boltapp.h, which the
 SDK's build tool writes before it compiles anything. modver.h includes it.
*/

#include <user.h>
#include <modver.h>
#include <wndmgr.h>
#include "calc.h"

#define __EXPORT __declspec(dllexport)

/*
 What the application is called and the picture it is drawn with. The window
 below takes both from here, and the file explorer reads them out of the file.
 The icon is a resource this module carries, by name: app.ini's [resources]
 attaches res/calculator.bmp to the file as AppIcon_Calculator.
 */
static RSRCTYPE_MANIFEST Manifest = {
	BOLTAPP_NAME, BOLTAPP_ICON, 0
};

/*
 What this file is, read out of the file itself by whoever asks without
 starting it; the file explorer's Properties is the one that does. The build
 checks that it is here and that it says what app.ini says. See modver.h.
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
 Called once, when the calculator is started. It opens its window here, at its
 own size; the window manager then calls __WindowProcedure2 for everything that
 happens in it. Returning from here does not end the application: it runs
 until its last window closes.
 */
__EXPORT int __modmain(void* Reserved) {
	CalcInit();

	Wndmgr_CreateWindow(10, 10, 180, 280, Manifest.Name,
		Wndmgr_LoadIcon(Manifest.IconName, 24), WNDMGRHNDFLAGS_NORESIZE);
	return 0;
}

/* Called once, when the calculator is closed. */
__EXPORT void __modhalt(void) {
	CalcHalt();
}

/*
 What the module describes about itself, one resource at a time. The system
 asks how many there are, then each one's type and its data. The return is a
 pointer or a small number, so it is pointer-width.
 */
__EXPORT WORDPTR __modres(unsigned long Index, unsigned long Query) {
	if (Query == MODRES_QUERY_COUNT)
		return 2; /* the manifest, and the version manifest */

	if (Index == 0) {
		if (Query == MODRES_QUERY_TYPE)
			return MODRES_RSRCTYPE_MANIFEST;
		if (Query == MODRES_QUERY_DATA)
			return (WORDPTR)&Manifest;
	}

	if (Index == 1) {
		if (Query == MODRES_QUERY_TYPE)
			return MODRES_RSRCTYPE_VERSION;
		if (Query == MODRES_QUERY_DATA)
			return (WORDPTR)&Version;
	}

	return 0; /* an index or a query this module does not know */
}
