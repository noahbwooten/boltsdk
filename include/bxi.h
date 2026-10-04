#pragma once
/*
bxi.h
BoltOS Usermode Include Declarations
Bolt Executable Installer Format

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"
#include "folders.h"
#include "registry.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 One file holding everything an application needs to be put on a machine: its
 modules and resources compressed, the registry values it wants, and the links
 it should be reachable by. A package is opened, not run, and what runs is the
 installer that reads it.

 Fixed records and no heap of strings, the same shape as applink.h and for the
 same reason: a reader takes a table in one read and every field is where the
 header says it is whatever the lengths turn out to be. Offsets are from the
 start of the file, so a table may be moved without any of the others knowing.

 A package is a file somebody else wrote. Every count, offset and length in it
 is a number to check before it is a number to allocate, and every member name
 is a name to refuse rather than to repair. See BxiSafeName.
*/

#define BXI_MAGIC   0x50495842  /* 'BXIP' */
#define BXI_VERSION 1

#define BXI_SUFFIX ".bxi"

/*
 The copy of the package kept inside the install folder, which is what removes
 the application later: what was written down is in the package, so the
 registry holds a pointer to it rather than a second copy of the list.
 */
#define BXI_PACKAGE_NAME "setup" BXI_SUFFIX

/* Where an installed application is written down, under HiveLocalMachine. One
   key per application, named by its AppId. */
#define BXI_INSTALLED_KEY "Software/Installed"

/* -- How long a field is -- */

/* An identity, which is also the folder under FOLDER_INSTALL and the key under
   BXI_INSTALLED_KEY. Short, since it is neither of those things' whole name. */
#define BXI_IDMAX 32

/* Something a person reads. */
#define BXI_TEXTMAX 64

/* A member's name inside the package, which is a path relative to the folder
   the application is installed into. Never absolute: joining it onto the
   install folder has to stay inside one. */
#define BXI_MEMBERMAX 128

/* An image by name, as an application's manifest names one. It travels inside
   the module this package installs, so an installed application brings its own
   picture with it. See modres.h. */
#define BXI_ICONMAX 32

/* A key path and a string value, held to what the registry itself holds. */
#define BXI_REGPATHMAX  192
#define BXI_REGVALUEMAX REG_STRINGMAX

/* -- What a package says about itself -- */

/*
 The application carries its own installer, which is a module inside the
 package named by CustomUi. Recorded and read, and this build installs with
 its own pages regardless: nothing here runs a packaged installer, and a
 package built for one is not refused for saying so.
 */
#define BXI_FLAG_CUSTOMUI 0x00000001

typedef struct _BXI_HEADER {
	WORD32 Magic;
	WORD32 Version;

	/* What the header itself measures, so a later version that appends fields
	   is read by this one up to the part it knows. */
	WORD32 HeaderSize;

	WORD32 Flags;

	char AppId[BXI_IDMAX];
	char DisplayName[BXI_TEXTMAX];
	char Publisher[BXI_TEXTMAX];
	char AppVersion[BXI_IDMAX];
	char IconName[BXI_ICONMAX];

	/* A member of this package, by name. Meaningless without
	   BXI_FLAG_CUSTOMUI. */
	char CustomUi[BXI_MEMBERMAX];

	WORD32 FileCount, FileOffset;
	WORD32 RegCount, RegOffset;
	WORD32 ShortcutCount, ShortcutOffset;

	WORD64 PayloadOffset, PayloadSize;

	/* Every member unpacked, added up. What the install will take on the
	   volume, and what is written down for the list of what is installed. */
	WORD64 InstalledSize;

	WORD32 Reserved[16];
}BXI_HEADER, * PBXI_HEADER;

/* -- One file to write out -- */

#define BXI_METHOD_STORE   0
#define BXI_METHOD_DEFLATE 1

typedef struct _BXI_FILE {
	/* Relative to the install folder. A folder in the path is made on the way
	   out, so a package may carry a tree. */
	char Name[BXI_MEMBERMAX];

	/* Where the member sits inside the payload, and how much of it there is
	   both ways round. Equal when the method is store. */
	WORD64 Offset;
	WORD64 Size;
	WORD64 Packed;

	/* Of the member unpacked, seeded and finished the way ArcCrc32 documents.
	   Checked after writing, since a package that unpacked to the wrong bytes
	   is worth knowing about before the application is run. */
	WORD32 Crc;

	WORD32 Method;

	/* What the file is marked as once it is written. Zero for an ordinary
	   file, which is what an application's own modules are: read-only is the
	   volume's lock, and locking what an update has to replace helps nobody. */
	WORD32 Attributes;

	WORD32 Flags;
}BXI_FILE, * PBXI_FILE;

/* -- One registry value to write -- */

/*
 Applied after the files, since a value naming a path should not be there
 before the path is. Removed on uninstall, which is why the package is kept:
 what to take out is read back out of it rather than written down twice.
 */
typedef struct _BXI_REGVALUE {
	WORD32 Hive;

	/* A REGTYPE_ from registry.h. REGTYPE_KEY makes the key and writes no
	   value, which is how a package asks for an empty key. */
	WORD32 Type;

	char Path[BXI_REGPATHMAX];
	char Name[BXI_TEXTMAX];

	/* One of the two is read, by type. A string value is in Value and every
	   number is in Number, rather than the number being spelled out. */
	char Value[BXI_REGVALUEMAX];
	WORD64 Number;

	WORD32 Flags;
	WORD32 Reserved;
}BXI_REGVALUE, * PBXI_REGVALUE;

/* -- One link to write -- */

#define BXI_SHORTCUT_ACTIONS 0
#define BXI_SHORTCUT_DESKTOP 1

/*
 Offered rather than made, and whether the offer starts ticked. A link that is
 neither is made without asking, which is what an application's own row in the
 Actions menu is: without it the application is installed and unreachable.
 */
#define BXI_SHORTCUT_OPTIONAL 0x00000001
#define BXI_SHORTCUT_DEFAULT  0x00000002

typedef struct _BXI_SHORTCUT {
	/* A member of this package, relative the same way BXI_FILE::Name is. The
	   link is written pointing at where that member ended up. */
	char Target[BXI_MEMBERMAX];

	char Name[BXI_TEXTMAX];
	char IconName[BXI_ICONMAX];

	WORD32 Location;
	WORD32 Flags;

	/* What a window should look like for a target that opens none of its own,
	   copied into the link. Ignored when W or H is zero, as APPLINK is. */
	WORD32 X, Y, W, H;
	WORD32 WindowFlags;

	WORD32 Reserved[4];
}BXI_SHORTCUT, * PBXI_SHORTCUT;

/* -- What is written down about an install -- */

/*
 The values under BXI_INSTALLED_KEY/<AppId>. Named here rather than spelled out
 twice, since the installer writes them and the list of installed applications
 reads them.

 The shortcuts made are recorded as whole paths, numbered, because which of
 them were made is the person's answer and is not in the package. The registry
 has no list, so a count and numbered names is what a list is here.
 */
#define BXI_VALUE_DISPLAYNAME "DisplayName"
#define BXI_VALUE_PUBLISHER   "Publisher"
#define BXI_VALUE_VERSION     "Version"
#define BXI_VALUE_ICONNAME    "IconName"
#define BXI_VALUE_INSTALLPATH "InstallPath"
#define BXI_VALUE_PACKAGE     "Package"
#define BXI_VALUE_SIZEONDISK  "SizeOnDisk"
#define BXI_VALUE_INSTALLDATE "InstallDate"
#define BXI_VALUE_CUSTOMUI    "CustomUi"
#define BXI_VALUE_SHORTCUTS   "Shortcuts"
#define BXI_VALUE_SHORTCUTN   "Shortcut%u"

/* -- What one reader holds at once -- */

/*
 Not format limits. The tables are counted in the header and a package may say
 any number; these are what is read into memory in one go, and a package
 claiming more than one of them is refused rather than half installed.
 */
#define BXI_FILES_MAX     64
#define BXI_REGVALUES_MAX 32
#define BXI_SHORTCUTS_MAX 8

/*
 The largest member this unpacks. Unpacking means holding what a member packs
 to and what it unpacks into at once, so the real cost of this is twice the
 number. A module is a long way under it.
 */
#define BXI_MEMBERBYTES (8 * 1024 * 1024)

#endif /* BOLTSDK_APIV_10 */
