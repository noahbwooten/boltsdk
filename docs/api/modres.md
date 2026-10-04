# modres.h and modver.h — what a module carries

`#include <user.h>` (which includes `modres.h`) · `#include <modver.h>` ·
exported by **user.dll** · link **`user.lib`**

A module's pictures and sounds travel inside its own file, attached after the
image by the build ([`[resources]` and `[sounds]` in
app.ini](../guide/project-file.md#resources)). They are read from the file,
never from the running image, so the shell can draw an application's icon in a
menu without starting it. The same file carries a version manifest, which the
file explorer's Properties reads without starting anything, and an SDK record,
which says which Bolt SDK the application was built against.

This page covers the calls that read all three, and what an application
declares. The guide to doing it is
[Resources and versions](../guide/resources-and-versions.md).

---

## What a module describes

The system asks a module about itself through the export `__modres`, one
resource at a time:

```c
__declspec(dllexport) WORDPTR __modres(unsigned long Index, unsigned long Query);
```

| `Query` | answer |
|---|---|
| `MODRES_QUERY_COUNT` | how many resources the module describes |
| `MODRES_QUERY_TYPE` | the type of resource `Index`: a `MODRES_RSRCTYPE_*` |
| `MODRES_QUERY_DATA` | a pointer to resource `Index`'s descriptor |

| Type | Descriptor | What it is |
|---|---|---|
| `MODRES_RSRCTYPE_MANIFEST` | `RSRCTYPE_MANIFEST` | the application's name and its icon |
| `MODRES_RSRCTYPE_VERSION` | `RSRCTYPE_VERSION` | the version manifest |
| `MODRES_RSRCTYPE_SERVICE` | `RSRCTYPE_SERVICE` | a service the module provides |
| `MODRES_RSRCTYPE_SCITEM2` | `RSRCTYPE_SCITEM2` | a page for System Control |
| `MODRES_RSRCTYPE_SCITEM` | `RSRCTYPE_SCITEM` | a System Control page, the original form |
| `MODRES_RSRCTYPE_IMAGE` | — | reserved |

```c
typedef struct _RSRCTYPE_MANIFEST {
    char Name[64];        /* what the application is called */
    char IconName[32];    /* a picture it carries, by name; empty for none */
    WORD32 Flags;         /* reserved */
} RSRCTYPE_MANIFEST;
```

A descriptor is handed back as a pointer with no length, so a type never
grows: a new layout is a new type, which an older reader skips.

## The version manifest

```c
typedef struct _RSRCTYPE_VERSION {
    WORD32 Magic;              /* MODVERSION_MAGIC */
    WORD32 Size;               /* sizeof(RSRCTYPE_VERSION) */
    WORD16 FileVersion[4];     /* major, minor, build, revision */
    WORD16 ProductVersion[4];
    char Description[64];
    char ProductName[64];
    char Copyright[64];
    char Language[32];
    char OriginalFilename[64];
} RSRCTYPE_VERSION;
```

Declared once, in its own section, and handed out by `__modres` as well —
which is also what stops the linker discarding it. `modver.h` has what that
takes. Built with the SDK, its macros take the application's numbers and
strings from `boltapp.h`, which the build writes from app.ini:

```c
#include <modver.h>

MODVERSION_PLACE static const RSRCTYPE_VERSION Version = {
    MODVERSION_HEADER,        /* magic, size, and the version twice */
    BOLTAPP_DESCRIPTION,
    MODVERSION_PRODUCT,       /* BOLTAPP_PRODUCT */
    MODVERSION_COPYRIGHT,     /* BOLTAPP_COPYRIGHT */
    MODVERSION_LANGUAGE,      /* BOLTAPP_LANGUAGE */
    BOLTAPP_FILENAME
};
```

The build refuses a file whose manifest is missing, is not this shape, or
gives a version or a file name app.ini does not.

## The SDK record

Every application built with `BOLTSDK_APIV` defined carries a
`BOLTSDK_RECORD` in a section named `.boltsdk`, saying which API version it
was built against. Nothing has to be written for it: `boltsdk.h` defines it.
The system reads it before starting an application, and refuses one built for
a newer SDK than it provides, with a message saying so. The system's own
modules carry no record.

---

### UserRes_LoadImage

```c
PVOID UserRes_LoadImage(PSTR Opt_ModulePath, PSTR Name,
                        PWORD32 Opt_OutWidth, PWORD32 Opt_OutHeight);
```

user.dll · export `UserRes_LoadImage1` · SDK 10

One picture out of a module, as 24-bit pixels ready for
[`Ge_DrawBuffer`](graphics.md#ge_drawbuffer), in a block the caller owns and
frees with [`User_Free`](user.md#user_free).

- `Opt_ModulePath` — the module to read. `NULL` is the calling application,
  and a name it does not carry is then looked for in the shell, which keeps
  what more than one module draws: the file type icons, the message box icons,
  `AppIcon_Placeholder`. A path is read alone, with no falling back.
- `Opt_OutWidth`, `Opt_OutHeight` — receive the size when not `NULL`.
- **Returns** the pixels, or `NULL` when there is no such picture.

### UserRes_Has

```c
BOOL UserRes_Has(PSTR Opt_ModulePath, PSTR Name);
```

user.dll · export `UserRes_Has1` · SDK 10

Whether a module carries a resource, picture or sound, without reading it.

### UserRes_AppIcon

```c
BOOL UserRes_AppIcon(PSTR Opt_ModulePath, PSTR Out, WORD32 Size);
```

user.dll · export `UserRes_AppIcon1` · SDK 10

The name of a module's own application icon: the first picture it carries
whose name begins `AppIcon_` (`MODRES_APPICON_PREFIX`). `FALSE` when it
carries none.

### UserRes_List

```c
WORD32 UserRes_List(PSTR Opt_ModulePath, PMODRES_ITEMINFO Out, WORD32 Max);
```

user.dll · export `UserRes_List1` · SDK 10

Every resource a module carries, in order, up to `Max` of them, described
without being read:

```c
typedef struct _MODRES_ITEMINFO {
    char Name[64];
    WORD32 Kind;            /* MODRES_KIND_PICTURE or MODRES_KIND_SOUND */
    WORD32 Width, Height;   /* a picture's size; a sound's rate and channels */
    WORD32 Bytes;           /* what it takes in the file */
} MODRES_ITEMINFO;
```

**Returns** how many there are, which can be more than `Max`; zero for a file
that carries none.

### UserRes_SelfPath

```c
BOOL UserRes_SelfPath(PSTR Out, WORD32 Size);
```

user.dll · export `UserRes_SelfPath1` · SDK 10

The file the calling application was started from, which is what `NULL` means
to the calls above. For an application that keeps files beside itself.

### UserRes_ShellPath

```c
BOOL UserRes_ShellPath(PSTR Out, WORD32 Size);
```

user.dll · export `UserRes_ShellPath1` · SDK 10

The shell's file, where the resources every module draws are kept.

### User_RegisterSound

```c
BOOL User_RegisterSound(PSTR Name);
```

user.dll · export `User_RegisterSound1` · SDK 10

Hands a sound the calling application carries to the mixer under its own
name, so [`User_PlaySound`](user.md#user_playsound) can play it. Call it once,
from `__modmain`; the samples are copied, so nothing has to stay alive. A name
another module has registered is refused, which is what stops an application
replacing `Sound_Error` for the whole machine.

### UserRes_Version

```c
BOOL UserRes_Version(PSTR Path, PRSRCTYPE_VERSION Out, PWORD32 Opt_OutLinked);
```

user.dll · export `UserRes_Version1` · SDK 10

The version manifest in whatever file a path names, read as bytes: nothing is
loaded or run, so any file may be asked. A manifest shorter than this build's
reads with the missing fields empty.

- `Opt_OutLinked` — receives when the file was linked, on the machine's clock,
  like the times in `USER_FILEINFO`; zero when the file does not say.
- **Returns** `FALSE` for a file that is not a module or declares no manifest.

### UserRes_SdkVersion

```c
WORD32 UserRes_SdkVersion(PSTR Path);
```

user.dll · export `UserRes_SdkVersion1` · SDK 10

Which Bolt SDK the module a path names was built against: the `BOLTSDK_APIV`
in its SDK record, such as 10. Zero for a module with no record — every one of
the system's own — and for a file that is not a module.
