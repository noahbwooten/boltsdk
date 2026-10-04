#pragma once
/*
registry.h
BoltOS Usermode Include Declarations
Registry Access Functions

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
#define UserReg_QueryString  UserReg_QueryString1
#define UserReg_QueryQword   UserReg_QueryQword1
#define UserReg_QueryDword   UserReg_QueryDword1
#define UserReg_QueryWord    UserReg_QueryWord1
#define UserReg_QueryBoolean UserReg_QueryBoolean1
#define UserReg_SetString    UserReg_SetString1
#define UserReg_SetQword     UserReg_SetQword1
#define UserReg_SetDword     UserReg_SetDword1
#define UserReg_SetWord      UserReg_SetWord1
#define UserReg_SetBoolean   UserReg_SetBoolean1
#define UserReg_CreateKey    UserReg_CreateKey1
#define UserReg_KeyExists    UserReg_KeyExists1
#define UserReg_DeleteKey    UserReg_DeleteKey1
#define UserReg_DeleteValue  UserReg_DeleteValue1
#define UserReg_RenameKey    UserReg_RenameKey1
#define UserReg_RenameValue  UserReg_RenameValue1
#define UserReg_ChildCount   UserReg_ChildCount1
#define UserReg_Enumerate    UserReg_Enumerate1
#define UserReg_Flush        UserReg_Flush1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/* -- Hives -- */

/*
 What the machine is, and what the user chose. The first is written by the
 shell and the kernel; an application may read it and may not write it, which
 is what keeps one application from changing the machine out from under the
 rest. HiveCurrentUser joins them when there is a user manager to mount it.
 */
#define REGHIVE_LOCALMACHINE       0
#define REGHIVE_LOCALCONFIGURATION 1

/* -- Value types -- */

#define REGTYPE_KEY     0
#define REGTYPE_STRING  1
#define REGTYPE_QWORD   2
#define REGTYPE_DWORD   3
#define REGTYPE_WORD    4
#define REGTYPE_BOOLEAN 5
#define REGTYPE_BINARY  6

/* -- Limits -- */

#define REG_NAMEMAX   64
#define REG_STRINGMAX 256
#define REG_PATHMAX   512

/* -- Reading -- */

/*
 Read one value. Each answers TRUE when the value was there and had the type
 asked for, and leaves the output alone otherwise, so a caller may seed the
 output with its default and use the answer only when it cares.

 Paths are separated by forward slashes and matched without regard to case,
 and the leading slash is optional: "Shell/Time" and "/shell/time" are one key.
 */
UMI_FUNCTION BOOL UserReg_QueryString(WORD32 Hive, PSTR Path, PSTR Name,
	PSTR Buffer, WORD32 BufferSize);
UMI_FUNCTION BOOL UserReg_QueryQword(WORD32 Hive, PSTR Path, PSTR Name,
	WORD64* Out);
UMI_FUNCTION BOOL UserReg_QueryDword(WORD32 Hive, PSTR Path, PSTR Name,
	WORD32* Out);
UMI_FUNCTION BOOL UserReg_QueryWord(WORD32 Hive, PSTR Path, PSTR Name,
	WORD16* Out);
UMI_FUNCTION BOOL UserReg_QueryBoolean(WORD32 Hive, PSTR Path, PSTR Name,
	WORD8* Out);

/* -- Writing -- */

/*
 Write one value, making the key and the value if either is missing. Answers
 TRUE when the value is now what was asked for.

 HiveLocalConfiguration is writable by an application, since a preference is
 the application's own. HiveLocalMachine is not: it is refused above the shell
 and the last error says why.
 */
UMI_FUNCTION BOOL UserReg_SetString(WORD32 Hive, PSTR Path, PSTR Name,
	PSTR Value);
UMI_FUNCTION BOOL UserReg_SetQword(WORD32 Hive, PSTR Path, PSTR Name,
	WORD64 Value);
UMI_FUNCTION BOOL UserReg_SetDword(WORD32 Hive, PSTR Path, PSTR Name,
	WORD32 Value);
UMI_FUNCTION BOOL UserReg_SetWord(WORD32 Hive, PSTR Path, PSTR Name,
	WORD16 Value);
UMI_FUNCTION BOOL UserReg_SetBoolean(WORD32 Hive, PSTR Path, PSTR Name,
	WORD8 Value);

/* -- Keys -- */

/* Make a key, and every key on the way to it. */
UMI_FUNCTION BOOL UserReg_CreateKey(WORD32 Hive, PSTR Path);

/* Answers whether a key is there, without making one. */
UMI_FUNCTION BOOL UserReg_KeyExists(WORD32 Hive, PSTR Path);

/* Remove a key and everything under it, or one value from a key. */
UMI_FUNCTION BOOL UserReg_DeleteKey(WORD32 Hive, PSTR Path);
UMI_FUNCTION BOOL UserReg_DeleteValue(WORD32 Hive, PSTR Path, PSTR Name);

/*
 Rename a key or a value in place, keeping its children and its position among
 its siblings. Answers FALSE when a sibling already has the name, so a caller
 may offer the new name and let this decide whether it is free.
 */
UMI_FUNCTION BOOL UserReg_RenameKey(WORD32 Hive, PSTR Path, PSTR NewName);
UMI_FUNCTION BOOL UserReg_RenameValue(WORD32 Hive, PSTR Path, PSTR Name,
	PSTR NewName);

/* -- Walking -- */

/*
 What a key holds, by position. Enumeration is by index rather than by a
 cursor, so nothing has to be opened or closed and a caller that stops early
 leaves nothing behind. Answers FALSE past the last child.

 Type is what the child is, and a REGTYPE_KEY is a key rather than a value.
 */
UMI_FUNCTION WORD32 UserReg_ChildCount(WORD32 Hive, PSTR Path);
UMI_FUNCTION BOOL UserReg_Enumerate(WORD32 Hive, PSTR Path, WORD32 Index,
	PSTR NameBuffer, WORD32 NameBufferSize, WORD32* Type);

/* Write the hives out now, rather than on the next tick. Wanted before
   something that will not come back, and not otherwise. */
UMI_FUNCTION VOID UserReg_Flush(VOID);

#endif /* BOLTSDK_APIV_10 */
