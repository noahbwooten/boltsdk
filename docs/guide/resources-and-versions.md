# Resources and versions

An application's file carries three things besides its code: the pictures and
sounds it uses, a version manifest saying what it is, and a record of the SDK
it was built for. All three are read out of the file without running it,
which is how the Actions menu draws an application's icon and the file
explorer shows its version before it has ever been started.

## Pictures and sounds

Listed in `app.ini`, and attached to the file by the build after it links:

```ini
[resources]
AppIcon_Hello = res/hello.bmp
Picture_Logo  = res/logo.bmp, no-compress

[sounds]
Sound_Done = res/done.wav
```

The code asks for them by name:

```c
PVOID Icon = Wndmgr_LoadIcon("AppIcon_Hello", 24);       /* 24 by 24 */

WORD32 W, H;
PVOID Logo = UserRes_LoadImage(NULL, "Picture_Logo", &W, &H);
Ge_DrawBuffer(20, 50, W, H, Logo);                      /* in MSG_PAINT */

User_RegisterSound("Sound_Done");                       /* once */
User_PlaySound("Sound_Done");
```

`NULL` for the module means the application itself; a name it does not carry
is then looked for in the shell, so the system's own pictures — the file type
icons, the message box icons, `AppIcon_Placeholder` — can be used by name too.
The pixels these hand back are the application's to free with `User_Free`
when it no longer draws them; an icon given to a window must last as long as
the window.

Pictures are `.bmp` files, 24-bit or paletted, uncompressed, with magenta
(255, 0, 255) as the transparent colour. Sounds are PCM `.wav` files.

### The application icon

An application's icon is a 32 by 32 picture whose name begins `AppIcon_`,
named by `icon` in `[application]`. It is drawn at 32 in the Actions menu, at
24 in the title bar and on the taskbar, and by the installer. Without one, the
system draws its placeholder.

## The manifest

`RSRCTYPE_MANIFEST` is the application's name and the name of its icon, handed
out by `__modres`. The window takes its title and icon from it:

```c
static RSRCTYPE_MANIFEST Manifest = {
    BOLTAPP_NAME, BOLTAPP_ICON, 0
};

__declspec(dllexport) int __modmain(void* Reserved) {
    Wndmgr_CreateWindow(100, 100, 360, 160, Manifest.Name,
        Wndmgr_LoadIcon(Manifest.IconName, 24), 0);
    return 0;
}
```

## The version manifest

What the file is, in a section of its own so it can be read as bytes:
description, version, product, copyright, language, and the name it was built
under. The file explorer's Properties shows it.

```c
#include <modver.h>

MODVERSION_PLACE static const RSRCTYPE_VERSION Version = {
    MODVERSION_HEADER,
    BOLTAPP_DESCRIPTION,
    MODVERSION_PRODUCT,
    MODVERSION_COPYRIGHT,
    MODVERSION_LANGUAGE,
    BOLTAPP_FILENAME
};
```

It must be returned from `__modres` as well, which is what stops the linker
throwing it away — see either example. The build checks the linked file and
refuses one whose manifest is missing, is the wrong shape, or gives a version
or file name other than app.ini's.

## boltapp.h

The `BOLTAPP_*` names above come from `boltapp.h`, which the build writes into
`out\<configuration>\obj` from `[application]` before compiling, and which
`modver.h` includes. The version and the names are said once, in `app.ini`,
and the file cannot disagree with it.

| | |
|---|---|
| `BOLTAPP_NAME` | `name` |
| `BOLTAPP_MODULE` | `module` |
| `BOLTAPP_FILENAME` | `<module>.bxf` |
| `BOLTAPP_DESCRIPTION` | `description` |
| `BOLTAPP_PRODUCT` | `product` |
| `BOLTAPP_COPYRIGHT` | `copyright` |
| `BOLTAPP_LANGUAGE` | `language` |
| `BOLTAPP_ICON` | `icon` |
| `BOLTAPP_VERSION_MAJOR`, `_MINOR`, `_BUILD`, `_REVISION` | `version` |
| `BOLTAPP_VERSION_STRING` | `version`, written out: `"1.0.0.0"` |

An application built some other way writes its own `boltapp.h` with the same
names.

## The SDK record

Every application built with `BOLTSDK_APIV` defined — every one the SDK
builds — carries a record of which API version it was built against, in a
section named `.boltsdk`. Nothing has to be written for it.

The system reads it before starting an application, and refuses one built for
a newer SDK than it provides: such an application would call functions the
system does not have. The shell says so in a message box rather than letting
the machine stop at the first missing function.
[`UserRes_SdkVersion`](../api/modres.md#userres_sdkversion) reads it.
