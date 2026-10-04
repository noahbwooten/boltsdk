#pragma once
/*
umbase.h
BoltOS Usermode Include Declarations
User-Mode Base Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

/* Which Bolt API is being built against, which every header below asks. */
#include "boltsdk.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define _countof(array) (sizeof(array) / sizeof(array[0]))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

typedef int INT, * PINT;
typedef unsigned int UINT, * PUINT;
typedef unsigned long long WORD64, *PWORD64;
typedef unsigned long WORD32, *PWORD32;
typedef unsigned short WORD16, *PWORD16;
typedef unsigned char WORD8, *PWORD8, BYTE, *PBYTE, BOOL, *PBOOL;
typedef char STR, *PSTR;
typedef void *PVOID;

// WORDPTR: an integer wide enough to hold a pointer. Shared with km.h, which
// describes the handle storage this API reads and writes.
#include "wordptr.h"

// A handle is an obfuscated pointer, so it has to be pointer-width.
typedef WORDPTR HANDLE;

#define VOID void
#define NULL 0
#define TRUE 1
#define FALSE 0

/*
 _fltused.

 A translation unit that touches floating point emits a reference to this, and
 the CRT is what normally answers it. There is no CRT in a hand-mapped image,
 so without this every module that does any arithmetic on a float fails to link
 naming a symbol that appears nowhere in the source.

 selectany rather than one chosen file: every unit including this defines it
 and the linker keeps one, so no module has to remember to carry it.

 "used" as well, and that is not belt and braces. Nothing in the source refers
 to this by name, so link-time optimisation drops it as dead, and the code
 generator then emits the reference that needs it. Release linked without it
 and Debug did not, which is exactly the shape of a difference nobody looks
 for.
 */
#if defined(__clang__)
__attribute__((used))
#endif
__declspec(selectany) int _fltused = 0;

#define UMI_FUNCTION __declspec(dllexport)

#endif /* BOLTSDK_APIV_10 */
