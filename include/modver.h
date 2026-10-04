#pragma once
/*
modver.h
BoltOS Usermode Include Declarations
Declaring A Module's Version Manifest

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "modres.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 What a module's main.c needs to declare its RSRCTYPE_VERSION. One of them, in
 the section a reader looks for, and handed out by __modres as well:

     MODVERSION_PLACE static const RSRCTYPE_VERSION Version = {
         MODVERSION_HEADER,
         "Calculator",           file description
         MODVERSION_PRODUCT,
         MODVERSION_COPYRIGHT,
         MODVERSION_LANGUAGE,
         "calc.bxf"              original filename
     };

 The section is read-only, so the manifest must be const. It has to be
 referenced from something that is kept, or a Release link discards it as
 unused; returning it from __modres is that, and the build refuses a module
 whose file does not carry one.
 */
#pragma section(".modver", read)
#define MODVERSION_PLACE __declspec(allocate(".modver"))


/*
 An application's own version, rather than the system's. It comes out of
 boltapp.h, which the SDK's build tool writes from the [application] section
 of the project's app.ini before compiling anything, so the version is said
 once, in the project, and the file it ends up in cannot disagree with it:

     BOLTAPP_VERSION_MAJOR, _MINOR, _BUILD and _REVISION   the four numbers
     BOLTAPP_PRODUCT, BOLTAPP_COPYRIGHT, BOLTAPP_LANGUAGE  the strings
     BOLTAPP_NAME, BOLTAPP_DESCRIPTION, BOLTAPP_FILENAME   for the manifests
     BOLTAPP_ICON                                          the icon's name

 An application built some other way writes a boltapp.h of its own with the
 same names. The tool checks the linked file against the project, the same
 way the system's build checks its own modules.
 */
#include <boltapp.h>

#define MODVERSION_NUMBERS \
	{ BOLTAPP_VERSION_MAJOR, BOLTAPP_VERSION_MINOR, \
	  BOLTAPP_VERSION_BUILD, BOLTAPP_VERSION_REVISION }

#define MODVERSION_PRODUCT   BOLTAPP_PRODUCT
#define MODVERSION_COPYRIGHT BOLTAPP_COPYRIGHT
#define MODVERSION_LANGUAGE  BOLTAPP_LANGUAGE


/* The first four fields. The product version is the file version: a module
   is never versioned apart from what it ships in. */
#define MODVERSION_HEADER \
	MODVERSION_MAGIC, sizeof(RSRCTYPE_VERSION), \
	MODVERSION_NUMBERS, MODVERSION_NUMBERS

#endif /* BOLTSDK_APIV_10 */
