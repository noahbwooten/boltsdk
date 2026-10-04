#pragma once
/*
modres.h
BoltOS Usermode Include Declarations
Resources Carried By A Module

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define UserRes_LoadImage  UserRes_LoadImage1
#define UserRes_AppIcon    UserRes_AppIcon1
#define UserRes_Has        UserRes_Has1
#define UserRes_SelfPath   UserRes_SelfPath1
#define UserRes_ShellPath  UserRes_ShellPath1
#define User_RegisterSound User_RegisterSound1
#define UserRes_List       UserRes_List1
#define UserRes_Version    UserRes_Version1
#define UserRes_SdkVersion UserRes_SdkVersion1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 A module carries the pictures and sounds it draws and plays, written into its
 own file after the end of the image. What is written is a resource bundle,
 exactly the format in res.h, with the footer below at the very end of the file
 so it can be found from there without walking anything.

 This works because HalLoadModule takes an image's length from the section
 table rather than from the file: what follows the last section is not part of
 the image and the loader never sees it. Nothing in the loader changed to allow
 this.

 A resource is read from the file every time, never from the mapped image, so
 the same call answers whether or not the module is running. That is what lets
 the shell draw an application's icon in a menu without starting it, which is
 the whole reason the bundle travels inside the module rather than beside it.
 */

#define MODRES_MAGIC   0x53455242 /* 'BRES' */
#define MODRES_VERSION 1

/* Written down rather than taken with sizeof, since the packer in Python has
   to agree with it and a field added on one side is a field added on both. */
#define MODRES_FOOTERSIZE 24

typedef struct _MODRES_FOOTER {
	WORD32 Magic;
	WORD32 Version;

	/* Both from the start of the file. */
	WORD64 Offset;
	WORD64 Size;
}MODRES_FOOTER, * PMODRES_FOOTER;

/*
 A bundle is read whole before anything in it is believed, so a module is
 refused rather than trusted when its footer says something the file cannot
 hold. Nothing here is larger than a few icons and one sound.
 */
#define MODRES_BUNDLEMAX (2 * 1024 * 1024)

/*
 One image out of a module's resources, as pixels the caller owns and frees.

 Opt_ModulePath names the module to read. NULL means the module asking, and a
 name that module does not carry is looked for in the shell, which is where
 what more than one module draws is kept: the file type icons, the message box
 icons, and the placeholder. An explicit path is read alone, since asking a
 named module for something is a different question from asking for a picture.
 */
UMI_FUNCTION PVOID UserRes_LoadImage(PSTR Opt_ModulePath, PSTR Name,
	PWORD32 Opt_OutWidth, PWORD32 Opt_OutHeight);

/*
 What an application's own icon is called in its own resources. A prefix rather
 than a whole name, since every module picks the rest of it, and written down
 here because a link that names no icon is drawn as what it points at and has
 to be able to find that picture.
 */
#define MODRES_APPICON_PREFIX "AppIcon_"

/* The name of a module's own application icon, out of the prefix above, or
   FALSE when it carries none. NULL is the module asking, as above. */
UMI_FUNCTION BOOL UserRes_AppIcon(PSTR Opt_ModulePath, PSTR Out, WORD32 Size);

/* Whether a module carries a resource, without reading it. */
UMI_FUNCTION BOOL UserRes_Has(PSTR Opt_ModulePath, PSTR Name);

/* The file the module asking was loaded from, which is what NULL resolves to
   above. FALSE when the caller is not a module the table knows. */
UMI_FUNCTION BOOL UserRes_SelfPath(PSTR Out, WORD32 Size);

/* The shell's file, which is where the resources every module draws live. */
UMI_FUNCTION BOOL UserRes_ShellPath(PSTR Out, WORD32 Size);

/*
 A sound out of this module's own resources, handed to the mixer under its own
 name. Called once, by whoever owns the sound, before anything plays it: the
 kernel copies the samples, so nothing has to stay alive afterwards.

 A name already registered by another module is refused. That is what stops an
 application replacing Sound_Error for the whole machine, and it is why the
 shell registers what it owns while it is starting, before an application can
 run at all.
 */
UMI_FUNCTION BOOL User_RegisterSound(PSTR Name);

/* -- What a module carries, listed -- */

/* What a resource is. The numbers are the bundle's own, from res.h. */
#define MODRES_KIND_PICTURE 0
#define MODRES_KIND_SOUND   1

/*
 One resource, described without being read. Width and Height are a picture's
 size in pixels, and a sound's sample rate and channel count, which is where
 the bundle keeps them. Bytes is what it takes up in the file: a picture may be
 stored compressed, and a sound is sixteen bit samples and nothing else.
 */
typedef struct _MODRES_ITEMINFO {
	char Name[64];
	WORD32 Kind;
	WORD32 Width, Height;
	WORD32 Bytes;
}MODRES_ITEMINFO, * PMODRES_ITEMINFO;

/*
 Every resource a module carries, in the order it carries them, up to Max.
 Answers how many there are, which can be more than Max, and zero for a file
 that carries none. Read from the file, like everything above; NULL is the
 module asking.
 */
UMI_FUNCTION WORD32 UserRes_List(PSTR Opt_ModulePath, PMODRES_ITEMINFO Out,
	WORD32 Max);

/* -- What a module is -- */

/*
 A module's version manifest: what the file is, which build of the system it
 came from, and what it was called when it was built. Declared in the module's
 own source beside its RSRCTYPE_MANIFEST and handed out by __modres as
 MODRES_RSRCTYPE_VERSION, the same as every other resource a module describes.
 modver.h has what a module needs to declare one.

 It is also put in a section of its own, MODVERSION_SECTION, and that is what
 lets it be read without the module. The section table sits at a fixed place in
 the file, so a reader finds the manifest by name and takes it as bytes:
 nothing is mapped and nothing is called. That is what lets the file explorer
 show it for a program that is not running, for a copy of one that has been
 renamed, and for the kernel, which cannot be loaded a second time to be asked.

 When the file was linked is not in here. The linker writes it into the header
 of every image, which is the same place this is read from.

 The strings are terminated inside their fields. A reader terminates them again
 regardless, since they came off a disk.
 */
#define MODVERSION_MAGIC   0x52455642 /* 'BVER' */
#define MODVERSION_SECTION ".modver"

typedef struct _RSRCTYPE_VERSION {
	WORD32 Magic;
	WORD32 Size;                /* sizeof, so a longer one later can be told */

	WORD16 FileVersion[4];      /* major, minor, build, revision */
	WORD16 ProductVersion[4];

	char Description[64];
	char ProductName[64];
	char Copyright[64];
	char Language[32];

	/* The name it was built under, which a rename or an install under
	   another name does not change. */
	char OriginalFilename[64];
}RSRCTYPE_VERSION, * PRSRCTYPE_VERSION;

/*
 The version manifest in whatever file a path names, read as bytes. Any file
 may be asked: one that is not a module, or a module that declares none,
 answers FALSE, and a caller that only wants to know whether there is one asks
 the same way.

 Opt_OutLinked is when the file was linked, from the image header, moved onto
 the machine's own clock so it reads like the times in USER_FILEINFO. Zero when
 the header gives no time.
 */
UMI_FUNCTION BOOL UserRes_Version(PSTR Path, PRSRCTYPE_VERSION Out,
	PWORD32 Opt_OutLinked);

/*
 Which Bolt SDK the module a path names was built against: the BOLTSDK_APIV
 its BOLTSDK_RECORD carries (see boltsdk.h), read out of the file the same way
 as the manifest above. Zero for a module that carries none, which is every
 one of the system's own, and for anything that is not a module.

 The process API asks this before it loads an application, and refuses one
 that wants a newer SDK than BOLTSDK_APIV_LATEST.
 */
UMI_FUNCTION WORD32 UserRes_SdkVersion(PSTR Path);

#endif /* BOLTSDK_APIV_10 */
