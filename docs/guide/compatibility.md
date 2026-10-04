# Compatibility

The point of the Bolt SDK is that an application built today keeps running on
every BoltOS after it. These are the rules that make that true, for the people
writing applications and for the people writing the system.

## Choosing a version

```c
#define BOLTSDK_APIV BOLTSDK_APIV_10      /* before the first include */
#include <user.h>
```

or `sdk = 10` in app.ini, which does the same. `BOLTSDK_APIV` is compared
against the `BOLTSDK_APIV_*` values: everything a header declares is in a section guarded by the version that
brought it,

```c
#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)
```

so asking for an older version hides what came later. Ask for the oldest
version that has what you need, and the application runs on the most systems.
Asking for a version older than 10, or newer than these headers know, is a
compile error. Left undefined, the SDK's headers use the latest.

## Numbered functions

Every function in the Bolt API is exported under a number, and the plain name
an application writes is mapped to the right one by the header:

```c
#define User_Allocate User_Allocate1
```

`User_Allocate1` is the first `User_Allocate`, and it never changes. If a later
SDK needs `User_Allocate` to take different arguments, it adds `User_Allocate2`
beside it, in a section guarded by that SDK's version, and the mapping becomes

```c
#if BOLTSDK_APIV >= BOLTSDK_APIV_11
#define User_Allocate User_Allocate2
#else
#define User_Allocate User_Allocate1
#endif
```

An application built against SDK 10 imports `User_Allocate1`, which every later
system still exports; one rebuilt against SDK 11 gets the new copy under the
same name. Either numbered copy can also be called by its number.

A name that already ends in a digit takes an underscore before the number, so
the version can still be read: `Thing32` would become `Thing32_1`. The three
functions that are already second copies — `User_GetFileSize2`,
`User_UpdateFileCursor2`, `User_TruncateFile2` — are called by those names.

## What never changes

Once a version is released:

- **No function's signature changes.** Not a parameter, not a type, not the
  return. A change is a new numbered copy.
- **No function is removed.** Its export stays in every later system.
- **No structure an application hands the system grows or changes.** A new
  layout is a new structure, with a new function or a new resource type that
  takes it, the way `RSRCTYPE_SCITEM2` stands beside `RSRCTYPE_SCITEM`.
- **No constant changes its value or meaning.** New flags, messages and error
  codes take numbers nobody has used; a retired one is left unused, never
  given to something else.
- **No message changes its meaning,** and the exports the system calls in an
  application — `__modmain`, `__modres`, `__WindowProcedure2` and the rest —
  keep their names and shapes.
- **Behaviour an application can rely on stays.** A fix that changes what a
  correct caller sees is a new copy, not a fix.

The C library subset (`memcpy`, `sprintf`, …) is outside this: those names are
the language's, and are not numbered. The system's own control calls are not
part of the API at all, and are not in the SDK.

## The SDK record

Every application records the SDK version it was built against. A system
refuses one that wants a newer SDK than it provides, and says so, rather than
letting it stop at the first function it lacks. An older system is the only
place an application can fail this way; a newer one always runs it.

## Naming

Every function is `Module_Function`: the prefix says which part of the system
it belongs to and so which library exports it — `User_`, `UserReg_`,
`UserRes_`, `UserVersion_`, `UserService_`, `UserConsole_`, `UserAtomic_` and
`Ge_` in user.dll; `Wndmgr_`; `CmnCtl_`; `CmnCtl2_`; `Shell_` and `ShellUi_` in
the shell; `Ps_`; `NetApi_`. A new function follows the same scheme.
