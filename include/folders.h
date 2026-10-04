#pragma once
/*
folders.h
BoltOS Usermode Include Declarations
Where things live

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "boltsdk.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 The folders more than one component has to agree on. Literals in one header
 rather than a path assembled in each of them: two components disagreeing about
 where the Actions folder is means a link written where nothing reads it, and
 that reads as a link that did not get written.
*/

/*
 A profile's own files. One profile and its name is a placeholder: multiple
 users are v3.30, and when they arrive this is where the name stops being a
 constant and every path below is built rather than concatenated.
 */
#define FOLDER_PROFILES        "/users"
#define FOLDER_PROFILE_DEFAULT "default"
#define FOLDER_PROFILE         FOLDER_PROFILES "/" FOLDER_PROFILE_DEFAULT

/*
 What is on the background, and what the Actions menu offers. Both belong to
 the person rather than to the machine, which is why they sit under the profile
 and not under /sys: a machine-wide list is not something either of them is.
 */
#define FOLDER_DESKTOP FOLDER_PROFILE "/desktop"
#define FOLDER_ACTIONS FOLDER_PROFILE "/actions"

/*
 What is opened once the desktop is up. The same links the other two folders
 hold, read the same way: dropping a link in here is the whole of adding a
 startup application, and taking it out is the whole of removing one.

 Under the profile with the rest, because what starts when somebody signs in is
 theirs. A machine-wide list is a different thing and is not this.
 */
#define FOLDER_STARTUP FOLDER_PROFILE "/startup"

/* Where an installed application is put, one folder per application named by
   the identity its package carries. See bxi.h. The machine's own programs'
   files live here too — the puzzle's pictures among them — so installs land
   beside them instead of in a folder of their own. */
#define FOLDER_INSTALL "/programs"

/* Where a package sits before it is installed. Nothing scans it; it is a place
   to put one, and an installer is reached by opening a package wherever it
   happens to be. */
#define FOLDER_PACKAGES "/sys/pkg"

#endif /* BOLTSDK_APIV_10 */
