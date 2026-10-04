# Types, errors and folders

The definitions every header shares: `umbase.h`, `umerr.h`, `folders.h`, and
the few constants from `user.h` that everything uses. All of them arrive with
`#include <user.h>`.

## Base types

```c
typedef unsigned char      BYTE, WORD8, BOOL;    /* BOOL is a byte */
typedef unsigned short     WORD16;
typedef unsigned long      WORD32;               /* 32 bits on this target */
typedef unsigned long long WORD64;
typedef int                INT;
typedef unsigned int       UINT;
typedef char               STR, *PSTR;
typedef void               *PVOID;
typedef unsigned long long WORDPTR;              /* holds a pointer */
typedef WORDPTR            HANDLE;

#define VOID  void
#define TRUE  1
#define FALSE 0
#define NULL  0
```

Each has a pointer type with a `P` in front: `PWORD32`, `PBOOL`, `PWORDPTR`.

- `WORD32` is 32 bits, and so is `long`, which stays 32 bits on BoltOS's x64
  target. Anything holding an address is a `WORDPTR`, never a `WORD32`.
- `BOOL` is one byte. A function declared `BOOL` that answers something other
  than `TRUE` or `FALSE` says so.

```c
#define _countof(array)  (sizeof(array) / sizeof(array[0]))
#define MIN(X, Y)        ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)        ((X) > (Y) ? (X) : (Y))
```

## Handles

```c
#define HANDLE_INVALID 0xFFFFFF69

#define HANDLETYPE_GENERIC   0
#define HANDLETYPE_FILE      1
#define HANDLETYPE_SHELLRES  2
#define HANDLETYPE_WINDOW    3
#define HANDLETYPE_SERVICE   4
#define HANDLETYPE_CONTROL   5
#define HANDLETYPE_CONSOLE   6
#define HANDLETYPE_MUTEX     7
#define HANDLETYPE_EVENT     8
```

See [Handles](user.md#handles). A few calls answer 0 rather than
`HANDLE_INVALID` when they fail — window and control creation among them — and
each says so.

## Errors

What [`User_GetLastError`](user.md#user_getlasterror) answers after a call that
failed. The value is left in place by calls that succeed.

| Code | Value | Meaning |
|---|---|---|
| `ERROR_SUCCESS` | 0x00 | nothing went wrong |
| `ERROR_PATH_NOT_FOUND` | 0x01 | there is nothing at that path |
| `ERROR_ACCESS_DENIED` | 0x02 | a lock, a hive the caller may not write, a file in use |
| `ERROR_INVALID_HANDLE` | 0x03 | not a handle, or not the caller's |
| `ERROR_INVALID_MODE` | 0x04 | the system cannot do that in its current state |
| `ERROR_MEMORY_FAILURE` | 0x05 | an allocation failed |
| `ERROR_OUT_OF_RESOURCES` | 0x06 | a fixed table is full: named objects, consoles |
| `ERROR_INVALID_ARGUMENT` | 0x07 | an argument out of range |
| `ERROR_INVALID_PATH` | 0x08 | a path that is not well formed, or is the wrong kind |
| `ERROR_INVALID_FILE_ID` | 0x09 | reserved |
| `ERROR_INVALID_CSR_MODIF` | 0x0A | reserved |
| `ERROR_INVALID_SVC_NAME` | 0x0B | no service by that name |
| `ERROR_INVALID_SVC_ID` | 0x0C | no service at that position |
| `ERROR_SVC_ALREADY_RUN` | 0x0D | the service is running already |
| `ERROR_SVC_ALREADY_STOP` | 0x0E | the service is stopped already |
| `ERROR_SVC_ALREADY_PAUSE` | 0x0F | the service is paused already |

## Folders

Where things live on the volume. Use these rather than writing the paths out,
so an application follows them if they move.

| Name | Path | What is there |
|---|---|---|
| `FOLDER_PROFILE` | `/users/default` | the user's own files |
| `FOLDER_DESKTOP` | `/users/default/desktop` | what is on the desktop |
| `FOLDER_ACTIONS` | `/users/default/actions` | the Actions menu, as links |
| `FOLDER_STARTUP` | `/users/default/startup` | links opened once the desktop is up |
| `FOLDER_INSTALL` | `/programs` | installed applications, one folder each |
| `FOLDER_PACKAGES` | `/sys/pkg` | packages waiting to be installed |
| `APPLINK_ACTIONS_PATH` | `/users/default/actions/` | the same as `FOLDER_ACTIONS`, with the slash |

`/sys` is the system's own and is marked system: an application reads it and
does not write it. An application keeps its files in its own folder under
`/programs`, or the user's under `/users/default`.

## The SDK's own definitions

From `boltsdk.h`; see [Compatibility](../guide/compatibility.md).

```c
#define BOLTSDK_APIV_10      10
#define BOLTSDK_APIV_LATEST  BOLTSDK_APIV_10
/* BOLTSDK_APIV — the version an application is built against */

#define BOLTSDK_RECORD_MAGIC   0x4B445342   /* 'BSDK' */
#define BOLTSDK_RECORD_SECTION ".boltsdk"

typedef struct _BOLTSDK_RECORD {
    unsigned int Magic;
    unsigned int Size;
    unsigned int ApiVersion;
    unsigned int Reserved[5];
} BOLTSDK_RECORD;
```

## Package format

`bxi.h` describes the `.bxi` installer format record by record, for a tool
that reads or writes packages. The SDK's `mkbxi.py` writes them; see
[Packaging](../guide/packaging.md).
