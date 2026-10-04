#pragma once
/*
user.h
BoltOS Usermode Include Declarations
User-Mode Core Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

/* The message-driven window procedure. Here rather than somewhere an
   application has to know to ask for, because every application includes this
   header and the point of the interface is that adopting it is a small step. */
#include "wndproc2.h"

/* What a module carries of its own. Here for the same reason as the header
   above: every module includes this one, and the resources a module draws are
   now part of it rather than of a bundle beside it. */
#include "modres.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define User_DestroyHandle               User_DestroyHandle1
#define User_IsHandleValid               User_IsHandleValid1
#define User_CreateHandle                User_CreateHandle1
#define User_GetDataForHandle            User_GetDataForHandle1
#define User_SetDataForHandle            User_SetDataForHandle1
#define UserAtomic_Read                  UserAtomic_Read1
#define UserAtomic_Increment             UserAtomic_Increment1
#define UserAtomic_Decrement             UserAtomic_Decrement1
#define UserAtomic_Add                   UserAtomic_Add1
#define UserAtomic_Exchange              UserAtomic_Exchange1
#define UserAtomic_CompareExchange       UserAtomic_CompareExchange1
#define User_CreateMutex                 User_CreateMutex1
#define User_TryMutex                    User_TryMutex1
#define User_AcquireMutex                User_AcquireMutex1
#define User_ReleaseMutex                User_ReleaseMutex1
#define User_MutexHeld                   User_MutexHeld1
#define User_DestroyMutex                User_DestroyMutex1
#define User_CreateEvent                 User_CreateEvent1
#define User_SetEvent                    User_SetEvent1
#define User_ClearEvent                  User_ClearEvent1
#define User_PollEvent                   User_PollEvent1
#define User_WaitEvent                   User_WaitEvent1
#define User_DestroyEvent                User_DestroyEvent1
#define User_Allocate                    User_Allocate1
#define User_Free                        User_Free1
#define User_Reallocate                  User_Reallocate1
#define User_LocalizeTime                User_LocalizeTime1
#define User_ZuluTime                    User_ZuluTime1
#define User_GetKTime                    User_GetKTime1
#define User_GetMsTime                   User_GetMsTime1
#define User_GetKeyState                 User_GetKeyState1
#define User_GetKeyboardState            User_GetKeyboardState1
#define User_BufferKeyState              User_BufferKeyState1
#define User_GetClickState               User_GetClickState1
#define User_GetButtonState              User_GetButtonState1
#define User_WasKeyPressed               User_WasKeyPressed1
#define User_WasKeyReleased              User_WasKeyReleased1
#define User_WasButtonPressed            User_WasButtonPressed1
#define User_WasButtonReleased           User_WasButtonReleased1
#define User_InputLost                   User_InputLost1
#define User_BufferClickState            User_BufferClickState1
#define User_GetMousePosition            User_GetMousePosition1
#define User_GetWheelTotal               User_GetWheelTotal1
#define User_BufferMousePosition         User_BufferMousePosition1
#define User_OpenFile                    User_OpenFile1
#define User_GetFileInfo                 User_GetFileInfo1
#define User_UpdateFileCursor            User_UpdateFileCursor1
#define User_GetFileSize                 User_GetFileSize1
#define User_TruncateFile                User_TruncateFile1
#define User_ReadFile                    User_ReadFile1
#define User_WriteFile                   User_WriteFile1
#define User_DeleteFile                  User_DeleteFile1
#define User_DeletePath                  User_DeletePath1
#define User_DeleteTree                  User_DeleteTree1
#define User_RenameFile                  User_RenameFile1
#define User_CopyFile                    User_CopyFile1
#define User_GetFileName                 User_GetFileName1
#define User_ValidateFilePath            User_ValidateFilePath1
#define User_GetAttributes               User_GetAttributes1
#define User_SetAttributes               User_SetAttributes1
#define User_OpenDirectory               User_OpenDirectory1
#define User_CreateDirectory             User_CreateDirectory1
#define User_ReadDirectory               User_ReadDirectory1
#define User_CountDirectory              User_CountDirectory1
#define User_FlushFileSystem             User_FlushFileSystem1
#define User_GetVolumeSpace              User_GetVolumeSpace1
#define User_HideExtensions              User_HideExtensions1
#define User_DisplayName                 User_DisplayName1
#define User_PlaySound                   User_PlaySound1
#define User_SoundReady                  User_SoundReady1
#define UserVersion_GetName              UserVersion_GetName1
#define UserVersion_GetProductName       UserVersion_GetProductName1
#define UserVersion_GetBuildString       UserVersion_GetBuildString1
#define UserVersion_GetBranchString      UserVersion_GetBranchString1
#define UserVersion_GetCopyright         UserVersion_GetCopyright1
#define UserVersion_GetShortVersion      UserVersion_GetShortVersion1
#define UserVersion_GetMajor             UserVersion_GetMajor1
#define UserVersion_GetMinor             UserVersion_GetMinor1
#define UserVersion_GetBuild             UserVersion_GetBuild1
#define UserVersion_GetRevision          UserVersion_GetRevision1
#define UserVersion_GetDate              UserVersion_GetDate1
#define UserVersion_GetTime              UserVersion_GetTime1
#define UserVersion_GetSuperShortVersion UserVersion_GetSuperShortVersion1
#define UserVersion_GetSku               UserVersion_GetSku1
#define UserVersion_GetArch              UserVersion_GetArch1
#define User_InsecureHashString          User_InsecureHashString1
#define User_GetLastError                User_GetLastError1
#define UserService_GetCount             UserService_GetCount1
#define UserService_GetName              UserService_GetName1
#define UserService_GetById              UserService_GetById1
#define UserService_GetByName            UserService_GetByName1
#define UserService_Start                UserService_Start1
#define UserService_Stop                 UserService_Stop1
#define UserService_Pause                UserService_Pause1
#define UserService_SetStartupState      UserService_SetStartupState1
#define UserService_GetState             UserService_GetState1
#define UserConsole_Open                 UserConsole_Open1
#define UserConsole_Close                UserConsole_Close1
#define UserConsole_Submit               UserConsole_Submit1
#define UserConsole_Collect              UserConsole_Collect1
#define UserConsole_Busy                 UserConsole_Busy1
#define UserConsole_TakeFlags            UserConsole_TakeFlags1
#define UserConsole_Prompt               UserConsole_Prompt1
#define UserConsole_Banner               UserConsole_Banner1

/* Numbered already, and called by that name: User_UpdateFileCursor2,
   User_GetFileSize2 and User_TruncateFile2 are the second copies of the three
   above them, and the names without a number mean the first. */
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define _countof(array) (sizeof(array) / sizeof(array[0]))

// definitions
typedef struct _UTM {
	unsigned short Year;
	unsigned char Month, Day,
		Hour, Minute, Second;
}UTM, *PUTM ;

#define HANDLE_INVALID 0xFFFFFF69

// enumerations for File Access ("WORD16 Permissions")
#define USER_FILEACCESS_READ 1
#define USER_FILEACCESS_WRITE 2
#define USER_FILEACCESS_DELETE 4
#define USER_FILEACCESS_CREATE 8

// enumerations for Cursor Manipulation ("WORD8 CursorMode")
#define USER_FILECSRPOSFROM_HEAD 0
#define USER_FILECSRPOSFROM_HERE 1
#define USER_FILECSRPOSFROM_TAIL 2

// enumerations for handle types
#define HANDLETYPE_GENERIC     0
#define HANDLETYPE_FILE        1
#define HANDLETYPE_SHELLRES    2
#define HANDLETYPE_WINDOW      3
#define HANDLETYPE_SERVICE     4

/* A retained control. Owned by cmnctl2, in the same handle table as everything
   else; see the note at the table's definition. */
#define HANDLETYPE_CONTROL     5

/* A command line session, owned by the kernel's console. */
#define HANDLETYPE_CONSOLE     6

/* A named mutex and a named event, owned by the kernel's table in ksync.c.
   Both hold a slot number rather than an address: an application cannot be
   handed the address of kernel storage and trusted to hand it back. */
#define HANDLETYPE_MUTEX       7
#define HANDLETYPE_EVENT       8

/*
 A file handle's slots. The node is sixty four bits and a slot is a word, so it
 is kept in two; it names the file itself rather than a position in a list, so
 nothing created or removed elsewhere can move it onto another file.
 */
#define HANDLETYPE_FILEDATA_NODELOW     0
#define HANDLETYPE_FILEDATA_NODEHIGH    1
#define HANDLETYPE_FILEDATA_FILEPOINTER 2
#define HANDLETYPE_FILEDATA_NODETYPE    3

#define HANDLETYPE_SHELLRES_KRESHANDLE 0
#define HANDLETYPE_SHELLRES_RESWIDTH   1
#define HANDLETYPE_SHELLRES_RESHEIGHT  2

#define HANDLETYPE_SERVICE_SERVICEID 0

#define HANDLETYPE_CONSOLE_SESSION 0

#define HANDLETYPE_MUTEX_SLOT 0
#define HANDLETYPE_EVENT_SLOT 0

/*
 The longest a user mode wait may run before it gives up, in milliseconds, and
 what a caller asking to wait indefinitely gets instead. There is one line of
 execution: a lock an application waits on cannot be released while it waits,
 so the wait is a formality and the answer is going to be no.

 A kernel caller may ask to wait until it is free, and stops the machine with a
 named lock when that runs out. An application may not, which is the difference
 between a bug being reported and an application being able to cause one.
 */
#define USER_WAIT_LONGEST 2000

// enumerations for service states
#define SVCSTATE_ERROR 0
#define SVCSTATE_PAUSED 1
#define SVCSTATE_STOPPED 2
#define SVCSTATE_FAILURE 3
#define SVCSTATE_RUNNING 4

// enumerations for module messages
#define MODMSG_PAINT   0x00
#define MODMSG_1SEC    0x01 
#define MODMSG_WIDGETS 0x02
#define MODMSG_USRDEF1 0x03
#define MODMSG_USRDEF2 0x04
#define MODMSG_USRDEF3 0x05
#define MODMSG_FBRSZ   0x06 // frame buffer resize

// Modres resource types
#define MODRES_RSRCTYPE_MANIFEST 0 // Application information manifest
#define MODRES_RSRCTYPE_SCITEM 1 // SystemControl Control Applet
#define MODRES_RSRCTYPE_IMAGE 2 // Bitmap Image
#define MODRES_RSRCTYPE_SERVICE 3 // Service
#define MODRES_RSRCTYPE_SCITEM2 4 // SystemControl Control Applet, message-driven
#define MODRES_RSRCTYPE_VERSION 5 // Version manifest; RSRCTYPE_VERSION, see modres.h

// Modres query types
#define MODRES_QUERY_TYPE 0
#define MODRES_QUERY_DATA 1
#define MODRES_QUERY_COUNT 2

/*
 What an application says about itself, and what it names its own window from.
 Nothing outside the application reads it any more: what the Actions menu
 offers is a link in APPLINK_ACTIONS_PATH, which carries a name and an icon of
 its own, and being listed stopped being the manifest's to decide.
 */
#define MANIFEST_FLAG_HIDDEN 0x00000001 /* reserved; nothing reads it */

typedef struct _RSRCTYPE_MANIFEST {
	char Name[64];

	/* An image this module carries, by name. Empty means no icon is drawn,
	   which is not the same as the resource being missing. See modres.h. */
	char IconName[32];

	WORD32 Flags;
}RSRCTYPE_MANIFEST, *PRSRCTYPE_MANIFEST;

/*
 The document an application was asked to open: void __modopen(char* Path).
 Called after __modmain, so the windows already exist, and optional, so one
 that does not export it is simply started with nothing open.
 */
#define MODOPEN_EXPORT_NAME "__modopen"

typedef struct _RSRCTYPE_SCITEM {
	BOOL IsEnabled;
	void* Procedure;
	char Name[32];
}RSRCTYPE_SCITEM, *PRSRCTYPE_SCITEM;

/*
 A page in the message-driven form. Its own resource type rather than fields
 appended to the one above, because __modres hands back a pointer with no
 length: a reader that assumed the longer layout would read past the end of a
 descriptor built to the shorter one. A reader that does not know this type
 skips it instead. See scitem2.h.
 */
typedef struct _RSRCTYPE_SCITEM2 {
	BOOL IsEnabled;
	void* Procedure2;
	char Name[32];

	/* What a System Control built before this contract calls, since it reads
	   the pointer and calls it without checking whether there is one. Null is
	   allowed, and means that host draws nothing for this page. */
	void* Procedure;
}RSRCTYPE_SCITEM2, *PRSRCTYPE_SCITEM2;

typedef struct _RSRCTYPE_SERVICE {
	char ServiceName[32];
	void* FunctionClock, * Shutdown, * Init, * Function1Sec;
}RSRCTYPE_SERVICE, *PRSRCTYPE_SERVICE;

// declarations

// handle functions
UMI_FUNCTION VOID User_DestroyHandle(HANDLE Handle);
UMI_FUNCTION BOOL User_IsHandleValid(HANDLE Handle);
UMI_FUNCTION HANDLE User_CreateHandle(BYTE HandleType, PWORD32 HandleData, int DataCount);
UMI_FUNCTION WORDPTR User_GetDataForHandle(HANDLE Handle, WORD32 Position);
UMI_FUNCTION VOID   User_SetDataForHandle(HANDLE Handle, WORD32 Position, WORDPTR Data);

/* -- Indivisible operations -- */

/*
 One instruction each, and none of them a call into the kernel. Here because an
 application counting something an interrupt also touches has no other way to
 say so, and because the alternative it reaches for otherwise is a read, a sum
 and a write that another line of execution can land in the middle of.

 Add and exchange answer with the value that was there before. Increment and
 decrement answer with the value after, because a caller of those is usually
 asking whether it reached zero.
 */
UMI_FUNCTION WORD32 UserAtomic_Read(volatile WORD32* Where);
UMI_FUNCTION WORD32 UserAtomic_Increment(volatile WORD32* Where);
UMI_FUNCTION WORD32 UserAtomic_Decrement(volatile WORD32* Where);
UMI_FUNCTION WORD32 UserAtomic_Add(volatile WORD32* Where, WORD32 Value);
UMI_FUNCTION WORD32 UserAtomic_Exchange(volatile WORD32* Where, WORD32 Value);

/* Writes Value only where what is there is still what Expected points at, and
   answers with whether it wrote. Expected is replaced by what was found either
   way, so a caller that loops does not read again at the top of it. */
UMI_FUNCTION BOOL   UserAtomic_CompareExchange(volatile WORD32* Where,
                                              PWORD32 Expected, WORD32 Value);

/* -- Mutexes -- */

/*
 Ownership of something across frames, by name, so that two applications reach
 the same one without either having to have created it. Opening a name that is
 not there creates it; opening one that is returns the same object.

 Recursive, so the owner taking it again succeeds and has to release it as many
 times as it took it.

 What this is for in a machine with one line of execution is saying that a thing
 is in use, not making a sequence indivisible: a second application only ever
 runs between this one's frames, never inside one. Single instance, a file open
 for editing, a long operation already under way.
 */
UMI_FUNCTION HANDLE User_CreateMutex(PSTR Name);

/* Never waits, and is the one to reach for. The answer is about this instant. */
UMI_FUNCTION BOOL   User_TryMutex(HANDLE Mutex);

/* Waits, capped at USER_WAIT_LONGEST however long is asked for. Answers false
   on expiry, which is a real outcome and not a failure of the call. */
UMI_FUNCTION BOOL   User_AcquireMutex(HANDLE Mutex, WORD32 TimeoutMs);

UMI_FUNCTION VOID   User_ReleaseMutex(HANDLE Mutex);
UMI_FUNCTION BOOL   User_MutexHeld(HANDLE Mutex);

/* Gives back the handle, and the slot with it once nothing holds the mutex.
   A mutex the caller is holding is released first, since the caller closing it
   is the last thing that knows the name. */
UMI_FUNCTION VOID   User_DestroyMutex(HANDLE Mutex);

/* -- Events -- */

/*
 Something that was started has finished. Auto-reset clears on the read that
 sees it, so one completion tells exactly one reader; manual stays set until
 cleared, which is what a state wants rather than a completion.

 Poll is what almost every application should use: it asks once and carries on
 with its frame. Wait is for the case where what sets the event is the kernel or
 a device rather than another application, since an application waiting on
 another application waits on something that cannot run until it returns.
 */
UMI_FUNCTION HANDLE User_CreateEvent(PSTR Name, BOOL AutoReset);
UMI_FUNCTION VOID   User_SetEvent(HANDLE Event);
UMI_FUNCTION VOID   User_ClearEvent(HANDLE Event);
UMI_FUNCTION BOOL   User_PollEvent(HANDLE Event);
UMI_FUNCTION BOOL   User_WaitEvent(HANDLE Event, WORD32 TimeoutMs);
UMI_FUNCTION VOID   User_DestroyEvent(HANDLE Event);

// memory functions
UMI_FUNCTION PVOID User_Allocate(WORD32 Size);
UMI_FUNCTION VOID User_Free(PVOID Pointer);
UMI_FUNCTION PVOID User_Reallocate(PVOID Existing, WORD32 Size);

// time functions
UMI_FUNCTION VOID User_LocalizeTime(WORD32 Time, PUTM OutTime);
UMI_FUNCTION VOID User_ZuluTime(WORD32 Time, PUTM OutPut);
UMI_FUNCTION WORD32 User_GetKTime(PWORD32 Opt_OutTime);

/*
 The millisecond clock, for measuring rather than for telling the time. It
 counts from an arbitrary point and wraps after seven weeks, so a difference
 between two readings is meaningful and a single reading is not.
 */
UMI_FUNCTION WORD32 User_GetMsTime(VOID);

// i/o functions
UMI_FUNCTION BOOL User_GetKeyState(WORD8 KeyCode);

/* All 256 key states in one crossing. Buffer must have room for 256 bytes.
   One call a frame instead of 256; see the note at the definition. */
UMI_FUNCTION VOID User_GetKeyboardState(PWORD8 Buffer);
UMI_FUNCTION VOID User_BufferKeyState(WORD8 KeyCode, BOOL State);
UMI_FUNCTION BOOL User_GetClickState(WORD8 ClickCode);

/*
 One button, held or not, numbered as the virtual keys are: MSGKEY_LBUTTON is
 the left and MSGKEY_RBUTTON the right. User_GetClickState answers for any button
 whatever it is asked, which made a right click a left click to every window.
 */
UMI_FUNCTION BOOL User_GetButtonState(WORD8 Button);

/*
 What happened since the last frame, rather than what is happening now. The
 states above are sampled once a frame from hardware that reports a thousand
 times a second, so a key struck and let go inside one frame moved twice and
 reads as never having moved. These are collected in the interrupt itself.

 Valid for one frame, so a caller looks every frame or not at all. Buttons are
 numbered as the virtual keys are, where one is the left, which is not how
 User_GetClickState numbers them.
 */
UMI_FUNCTION BOOL User_WasKeyPressed(WORD8 KeyCode);
UMI_FUNCTION BOOL User_WasKeyReleased(WORD8 KeyCode);
UMI_FUNCTION BOOL User_WasButtonPressed(WORD8 Button);
UMI_FUNCTION BOOL User_WasButtonReleased(WORD8 Button);

/* Edges dropped between this frame and the last. Zero unless the machine
   stopped for longer than the ring is deep. */
UMI_FUNCTION WORD32 User_InputLost(VOID);
UMI_FUNCTION VOID User_BufferClickState(WORD8 ClickCode, BOOL State);
UMI_FUNCTION VOID User_GetMousePosition(PWORD16 MouseX, PWORD16 MouseY);
UMI_FUNCTION long User_GetWheelTotal(VOID);
UMI_FUNCTION VOID User_BufferMousePosition(WORD16 MouseX, WORD16 MouseY);

/* -- Files -- */

/* What a directory entry is, and what a handle was opened on. */
#define USER_FILETYPE_NONE 0
#define USER_FILETYPE_FILE 1
#define USER_FILETYPE_DIR  2

/*
 What a file is, apart from what is in it. Owner, Mode and Frame are carried
 from the volume and are reserved: nothing owns a file yet and nothing enforces
 them, and the room is there so that adding it later changes no layout.

 The times are seconds since 1970 by the machine's own clock, which keeps local
 time rather than UTC, so User_LocalizeTime turns them straight into a date that
 agrees with the taskbar. Accessed took the word
 that used to be Reserved, which is why it is narrower than the other two and
 why nothing built before it has to change. Zero, in any of them, is a file on
 a volume that never recorded it.
 */
typedef struct _USER_FILEINFO {
	WORD32 Type;
	WORD32 Flags;

	WORD64 Size;
	WORD64 Created;
	WORD64 Modified;

	WORD32 Owner;
	WORD32 Mode;
	WORD32 Frame;
	WORD32 Accessed;
}USER_FILEINFO, * PUSER_FILEINFO;

/*
 What Flags carries. Neither lock is a permission and the way past either is to
 take the attribute off. Read-only refuses a write from anybody, the machine
 included. System is read-only and hidden together as an application sees it,
 and neither as the machine does: a system file is the machine's own, so it
 goes on writing what nothing here may write.
 */
#define USER_ATTR_NONE     0x00
#define USER_ATTR_READONLY 0x01
#define USER_ATTR_HIDDEN   0x02
#define USER_ATTR_SYSTEM   0x04

/* Hidden asks a listing not to show a file. System asks the same and is not
   granted by the same answer, which is what protected means. */
#define USER_ATTRLOCKED(a) \
	(((a) & (USER_ATTR_READONLY | USER_ATTR_SYSTEM)) != 0)
#define USER_ATTRCONCEALED(a) \
	(((a) & (USER_ATTR_HIDDEN | USER_ATTR_SYSTEM)) != 0)

UMI_FUNCTION HANDLE User_OpenFile(PSTR FilePath, WORD16 Permissions);
UMI_FUNCTION BOOL   User_GetFileInfo(HANDLE File, PUSER_FILEINFO Out);

/*
 Sizes and positions, at the width of the volume. A file is sixty four bits
 from the disk up, so these are what a new caller uses; the three without the
 2 are the same calls seen through a thirty-two bit window and are kept for
 everything written before there was another choice.

 The narrow pair answer 0xFFFFFFFF for a position or a size that will not fit,
 which is the value they already answered a bad handle with. A file that large
 cannot be worked with through them either way, so the two failures are one.
 */
#define USER_FILESIZE_ERROR 0xFFFFFFFFFFFFFFFFULL
#define USER_FILECSR_ERROR  0xFFFFFFFFFFFFFFFFULL

UMI_FUNCTION WORD64 User_UpdateFileCursor2(HANDLE File, WORD8 CursorMode, WORD64 NewPos);
UMI_FUNCTION WORD64 User_GetFileSize2(HANDLE File);
UMI_FUNCTION BOOL   User_TruncateFile2(HANDLE File, WORD64 Size);

UMI_FUNCTION WORD32 User_UpdateFileCursor(HANDLE File, WORD8 CursorMode, WORD32 NewPos);
UMI_FUNCTION WORD32 User_GetFileSize(HANDLE File);
UMI_FUNCTION BOOL   User_TruncateFile(HANDLE File, WORD32 Size);

/*
 Both move the cursor by what they moved, so a run of them walks the file. The
 cursor they move is the wide one, so a run of them walks a file of any size;
 what is thirty-two bits here is how much one call carries, which is a buffer
 rather than a file.
 */
UMI_FUNCTION WORD32 User_ReadFile(HANDLE File, PVOID Buffer, WORD32 BufferSize, WORD32 BytesToRead);
UMI_FUNCTION WORD32 User_WriteFile(HANDLE File, PVOID Buffer, WORD32 BytesToWrite);

UMI_FUNCTION VOID   User_DeleteFile(HANDLE File);
UMI_FUNCTION BOOL   User_DeletePath(PSTR Path);

/*
 A path and everything under it, for a folder somebody meant to be rid of. The
 volume refuses a directory that is not empty, so this is the walk that empties
 one; a file is deleted as User_DeletePath would.

 Refuses the whole tree rather than taking part of it when anything in it is
 read-only or the machine's own.
 */
#define USER_DELETETREE_MAXDEPTH 16
#define USER_NAMEMAX 256

UMI_FUNCTION BOOL   User_DeleteTree(PSTR Path);
UMI_FUNCTION BOOL   User_RenameFile(PSTR Path, PSTR NewPath);
UMI_FUNCTION BOOL   User_CopyFile(PSTR Path, PSTR NewPath);

/* The whole path of what a handle is open on. */
UMI_FUNCTION WORD32 User_GetFileName(HANDLE File, PSTR Buffer, WORD32 BufferSize);

UMI_FUNCTION BOOL   User_ValidateFilePath(PSTR Path);

/* -- Attributes -- */

/* What a path carries, and USER_ATTR_NONE when there is nothing there. */
UMI_FUNCTION WORD32 User_GetAttributes(PSTR Path);

/* Replaced whole, so a caller reads them, changes the bit it means and writes
   them back. Refused on nothing: this is what takes either lock off, system
   included, and is deliberately the one call the marks do not hold against. */
UMI_FUNCTION BOOL   User_SetAttributes(PSTR Path, WORD32 Attributes);

/* -- Directories -- */

UMI_FUNCTION HANDLE User_OpenDirectory(PSTR Path);
UMI_FUNCTION BOOL   User_CreateDirectory(PSTR Path);

/* One entry by position, FALSE past the last, so a caller walks upward rather
   than asking how many there are first. */
UMI_FUNCTION BOOL   User_ReadDirectory(HANDLE Directory, WORD32 Index,
	PSTR NameOut, WORD32 NameMax, PWORD32 TypeOut);
UMI_FUNCTION WORD32 User_CountDirectory(HANDLE Directory);

/* Push anything held back out to the volume. */
UMI_FUNCTION VOID   User_FlushFileSystem(VOID);

/*
 The size of the volume a path sits on and how much of it is free, in bytes.
 Asked of a path rather than of the machine, since a machine may carry more
 than one volume; "/" names the one every machine boots from. FALSE when the
 volume could not say, with both outputs left at zero.
 */
UMI_FUNCTION BOOL   User_GetVolumeSpace(PSTR Path, PWORD64 Total, PWORD64 Free);

/* -- Names as they are shown -- */

/*
 Whether a listing is meant to leave extensions off, from Folder Options. Read
 from the registry every time, so ask it while building a list rather than
 while painting one.
 */
UMI_FUNCTION BOOL   User_HideExtensions(VOID);

/*
 One leaf name as somebody sees it. The extension comes off when the option
 above is set, never from a directory, and never when the dot is the first
 character, so a file called .config keeps its name.

 A link is trimmed either way: what a link is called is the thing it stands
 for, and .bal is how that is written down rather than part of the name.
 */
UMI_FUNCTION VOID   User_DisplayName(PSTR Name, BOOL IsDirectory, PSTR Out,
	WORD32 Size);

/*
 Sound. The name is a resource in the shell bundle, such as "Sound_Error", and
 that is the whole of it: a sound is short, it is resident already, and it
 finishes on its own, so there is no handle to hold and nothing to free.

 Play returns FALSE when there is no device, when the name is not in the
 bundle, or when every voice is busy. None of the three is worth acting on.
 Ready is for anything that would rather leave a control out than offer one
 that does nothing.
 */
UMI_FUNCTION BOOL   User_PlaySound(PSTR Name);
UMI_FUNCTION BOOL   User_SoundReady(VOID);

// system info functions
UMI_FUNCTION PSTR   UserVersion_GetName(VOID);
UMI_FUNCTION PSTR   UserVersion_GetProductName(VOID);
UMI_FUNCTION PSTR   UserVersion_GetBuildString(VOID);
UMI_FUNCTION PSTR   UserVersion_GetBranchString(VOID);
UMI_FUNCTION PSTR   UserVersion_GetCopyright(VOID);
UMI_FUNCTION PSTR   UserVersion_GetShortVersion(VOID);
UMI_FUNCTION WORD32 UserVersion_GetMajor(VOID);
UMI_FUNCTION WORD32 UserVersion_GetMinor(VOID);
UMI_FUNCTION WORD32 UserVersion_GetBuild(VOID);
UMI_FUNCTION WORD32 UserVersion_GetRevision(VOID);
UMI_FUNCTION WORD32 UserVersion_GetDate(VOID);
UMI_FUNCTION WORD32 UserVersion_GetTime(VOID);
UMI_FUNCTION PSTR   UserVersion_GetSuperShortVersion(VOID);
UMI_FUNCTION PSTR   UserVersion_GetSku(VOID);
UMI_FUNCTION PSTR   UserVersion_GetArch(VOID);

// misc functions
UMI_FUNCTION WORD32 User_InsecureHashString(PSTR Str);
UMI_FUNCTION WORD32 User_GetLastError(VOID);

// service functions
UMI_FUNCTION WORD32 UserService_GetCount(VOID);
UMI_FUNCTION PSTR   UserService_GetName(WORD32 Id);
UMI_FUNCTION HANDLE UserService_GetById(WORD32 Id);
UMI_FUNCTION HANDLE UserService_GetByName(PSTR Name);
UMI_FUNCTION VOID   UserService_Start(HANDLE Service);
UMI_FUNCTION VOID   UserService_Stop(HANDLE Service);
UMI_FUNCTION VOID   UserService_Pause(HANDLE Service);
UMI_FUNCTION VOID   UserService_SetStartupState(HANDLE Service, BOOL State);
UMI_FUNCTION BOOL   UserService_GetState(HANDLE Service);
/*
 console functions

 A session is a conversation with the command line, the same one telnet
 answers. Output is collected rather than pushed, so an application drains it
 on its own clock instead of being called back from the kernel's.

 Nothing here involves the network. A command that does is registered by the
 network, so a machine without one has a console with fewer commands.
 */
UMI_FUNCTION HANDLE UserConsole_Open(VOID);
UMI_FUNCTION VOID   UserConsole_Close(HANDLE Console);

/* Run one line. What it produced is read back with UserConsole_Collect, which
   may need more than one call and may need a later frame. */
UMI_FUNCTION VOID   UserConsole_Submit(HANDLE Console, PSTR Line);

/* Take what has been produced since the last call. Returns how many bytes,
   which may be zero, and does not terminate the buffer. */
UMI_FUNCTION WORD32 UserConsole_Collect(HANDLE Console, PSTR Buffer, WORD32 Capacity);

/* Whether the last line is still being answered. A command that needs a reply
   from the network finishes in a later frame than the one that started it. */
UMI_FUNCTION BOOL   UserConsole_Busy(HANDLE Console);

/* What a command asked for, read once and cleared. USERCONSOLE_CLEAR means the
   screen, and USERCONSOLE_CLOSE means the session is finished. */
UMI_FUNCTION WORD32 UserConsole_TakeFlags(HANDLE Console);

/* The prompt every front end draws, and the greeting a session opens with. */
UMI_FUNCTION PSTR   UserConsole_Prompt(VOID);
UMI_FUNCTION PSTR   UserConsole_Banner(VOID);

#define USERCONSOLE_CLOSE   0x01
#define USERCONSOLE_CLEAR   0x02

#endif /* BOLTSDK_APIV_10 */
