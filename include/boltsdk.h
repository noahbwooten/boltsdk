#pragma once
/*
boltsdk.h
BoltOS Usermode Include Declarations
Bolt SDK API Versions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

/*
 Which version of the Bolt API an application is built against. An application
 names the version before its first include, and every header compares it
 against the BOLTSDK_APIV_* values:

     #define BOLTSDK_APIV BOLTSDK_APIV_10
     #include <user.h>

 What a version promises is that nothing in it changes. A function is never
 given another signature and never removed; a structure an application hands
 the system never grows. A change is a second copy beside the first, with the
 next number: User_Allocate1 is the first User_Allocate, and a User_Allocate2
 would be declared in a section of its own, guarded by the version that
 brought it. Which copy the plain name means is decided by BOLTSDK_APIV, so an
 application asking for an older SDK gets the older copy under the same name,
 and an application built years ago calls an export that is still there.

 Each header carries a table of the names it declares, mapping each to the
 numbered export the requested version gets:

     #define User_Allocate User_Allocate1

 and wraps what it declares in the version that brought it:

     #if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

 Left undefined, nothing is versioned. The tables are not applied, so the
 plain names call the system's plain exports, and what the system keeps to
 itself -- the KmCtl_ calls and the structures behind them -- is declared as
 well. That is how the system builds itself. It is not something an
 application should do, since the plain exports are the system's own and are
 free to change, which is why the SDK's copy of this header defines
 BOLTSDK_APIV as the latest when nothing else has.

 A name that already ends in a digit takes an underscore before its number,
 so that the version stays readable: KmCtl_Random32 would be KmCtl_Random32_1.
 */
/* This is the SDK's copy. */
#define BOLTSDK_PACKAGE 1

#define BOLTSDK_APIV_10      10
#define BOLTSDK_APIV_LATEST  BOLTSDK_APIV_10

/* Defined only in the SDK's copy, by the script that makes it. */
#if defined(BOLTSDK_PACKAGE) && !defined(BOLTSDK_APIV)
#define BOLTSDK_APIV BOLTSDK_APIV_LATEST
#endif

#ifdef BOLTSDK_APIV
#if BOLTSDK_APIV < BOLTSDK_APIV_10
#error BOLTSDK_APIV is older than the first Bolt SDK, which is BOLTSDK_APIV_10
#endif

#if BOLTSDK_APIV > BOLTSDK_APIV_LATEST
#error BOLTSDK_APIV names a newer Bolt SDK than these headers; install that SDK
#endif
#endif

/*
 Which SDK a module was built against, written into a section of its own so it
 can be read out of the file without loading it, the same way the version
 manifest is (see modres.h). An application built against a newer SDK than
 the system provides calls exports the system does not have, so the system
 refuses to start it and says why, rather than stopping at the first call that
 was never bound.

 Every application built with BOLTSDK_APIV defined carries one, without a line
 of its own: it is defined here, once per module, and kept by the linker even
 though nothing refers to it. The system's own modules carry none, and are
 never refused.
 */
#define BOLTSDK_RECORD_MAGIC   0x4B445342 /* 'BSDK' */
#define BOLTSDK_RECORD_SECTION ".boltsdk"

typedef struct _BOLTSDK_RECORD {
	unsigned int Magic;
	unsigned int Size;          /* sizeof, so a longer one later can be told */
	unsigned int ApiVersion;    /* the BOLTSDK_APIV it was built with */
	unsigned int Reserved[5];
}BOLTSDK_RECORD, * PBOLTSDK_RECORD;

#ifdef BOLTSDK_APIV
#pragma section(".boltsdk", read)

#if defined(__clang__)
__attribute__((used))
#endif
__declspec(allocate(".boltsdk")) __declspec(selectany)
const BOLTSDK_RECORD BoltSdk_Record = {
	BOLTSDK_RECORD_MAGIC, sizeof(BOLTSDK_RECORD), BOLTSDK_APIV, { 0 }
};
#endif
