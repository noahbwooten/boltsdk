#pragma once
/*
psapi.h
BoltOS Usermode Include Declarations
The process API

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
#define Ps_CreateProcess     Ps_CreateProcess1
#define Ps_StartProcess      Ps_StartProcess1
#define Ps_KillProcess       Ps_KillProcess1
#define Ps_SuspendProcess    Ps_SuspendProcess1
#define Ps_ResumeProcess     Ps_ResumeProcess1
#define Ps_IsSuspended       Ps_IsSuspended1
#define Ps_IsProcess         Ps_IsProcess1
#define Ps_IsApplication     Ps_IsApplication1
#define Ps_GetCurrentProcess Ps_GetCurrentProcess1
#define Ps_FindProcessByBase Ps_FindProcessByBase1
#define Ps_GetProcessBase    Ps_GetProcessBase1
#define Ps_GetProcessMax     Ps_GetProcessMax1
#define Ps_GetProcessPath    Ps_GetProcessPath1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 psapi: what starts, stops and holds an application. A process is a loaded
 module with an entry point, and this is the only sanctioned way to make one or
 end one. The kernel keeps the module table; nothing here holds state of its own.

     WORD32 Id = Ps_CreateProcess("/sys/apps/calc.bxf");
     Ps_StartProcess(Id);
     ...
     Ps_KillProcess(Id, TRUE);

 Creating and starting are separate calls so that a caller can look at the
 process between the two, which is what lets the shell tell a window the entry
 point opened from one it has to supply itself.
 */

/* Exported when building the library, imported by everything else. _PSAPI is
   defined only for this module's own sources; without the split the library
   would declare its own functions as imported and then define them. */
#ifdef _PSAPI
#define PSAPI_EXPORT __declspec(dllexport)
#else
#define PSAPI_EXPORT __declspec(dllimport)
#endif

/* No process. Slot zero is a real one, so a failure cannot be reported as
   zero the way a pointer would be. */
#define PS_INVALID 0xFFFFFFFF

/*
 Load an image and give it a slot in the module table, without running it.
 Returns the process identifier, or PS_INVALID. The path is what the module is
 named by afterwards, so it is the path that was opened and not a friendly name.
 */
PSAPI_EXPORT WORD32 Ps_CreateProcess(PSTR Path);

/*
 Run the entry point, with the process as the active module so that everything
 it allocates and every window it opens is billed to it. Starting one that is
 already running enters it again, which is how it opens a second window.
 */
PSAPI_EXPORT BOOL Ps_StartProcess(WORD32 Process);

/*
 End it, and release everything the module table records against it. Graceful
 calls __modhalt first, which is what an application expects to be able to save
 from; without it the process is dropped where it stands.
 */
PSAPI_EXPORT BOOL Ps_KillProcess(WORD32 Process, BOOL Graceful);

/*
 Stop calling the process's clock, and start again. Its windows stay on screen
 showing whatever they last drew, since suspending stops it running rather than
 taking anything away from it.
 */
PSAPI_EXPORT BOOL Ps_SuspendProcess(WORD32 Process);
PSAPI_EXPORT BOOL Ps_ResumeProcess(WORD32 Process);
PSAPI_EXPORT BOOL Ps_IsSuspended(WORD32 Process);

/* Whether the identifier names a process at all, which every other call here
   asks first. */
PSAPI_EXPORT BOOL Ps_IsProcess(WORD32 Process);

/*
 An application rather than one of the core modules. Only an application can be
 suspended or killed, so this is what a caller asks before offering either.
 */
PSAPI_EXPORT BOOL Ps_IsApplication(WORD32 Process);

/* The process that is running, for one that needs to name itself. */
PSAPI_EXPORT WORD32 Ps_GetCurrentProcess(VOID);

/* Where the image was loaded, for a caller holding a base rather than an
   identifier. Returns PS_INVALID when no process has that base. */
PSAPI_EXPORT WORD32 Ps_FindProcessByBase(PVOID Base);
PSAPI_EXPORT PVOID Ps_GetProcessBase(WORD32 Process);

/* How many slots there are, so a caller can walk them. Loaded or not is
   Ps_IsProcess, since the table is sparse. */
PSAPI_EXPORT WORD32 Ps_GetProcessMax(VOID);

/*
 The file a process was loaded from. What a module carries of its own lives in
 that file, so this is what a caller passes to Wndmgr_LoadIconFrom to draw
 another application's icon. See modres.h.
 */
PSAPI_EXPORT BOOL Ps_GetProcessPath(WORD32 Process, PSTR Out, WORD32 Size);

#endif /* BOLTSDK_APIV_10 */
