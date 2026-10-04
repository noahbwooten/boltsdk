# registry.h — the registry

`#include <registry.h>` · exported by **user.dll** · link **`user.lib`**

The registry is a tree of keys holding typed values, kept on the volume and
written back on the system's own clock. It is where the system keeps its
settings and where an application keeps its preferences.

## Hives

```c
#define REGHIVE_LOCALMACHINE       0   /* what the machine is */
#define REGHIVE_LOCALCONFIGURATION 1   /* what the user chose */
```

`REGHIVE_LOCALCONFIGURATION` is an application's to read and write: a
preference is the application's own. Keep yours under a key named for the
application, such as `Applications/MyApp`.

`REGHIVE_LOCALMACHINE` is the machine's. An application may read it and should
treat it as read-only: a write from an application's entry point or clock is
refused with `ERROR_ACCESS_DENIED`, and a later system may refuse it
everywhere. An installed package may write values under `Software/<appid>` as
it is installed; see [Packaging](../guide/packaging.md#registry-values).

## Paths, names and types

A path is keys separated by `/`, matched without regard to case, with an
optional leading slash: `Shell/Time` and `/shell/time` are one key. A name
picks one value within a key.

```c
#define REGTYPE_KEY     0   /* a key, when enumerating */
#define REGTYPE_STRING  1
#define REGTYPE_QWORD   2   /* 64 bits */
#define REGTYPE_DWORD   3   /* 32 bits */
#define REGTYPE_WORD    4   /* 16 bits */
#define REGTYPE_BOOLEAN 5   /* 8 bits, zero or not */
#define REGTYPE_BINARY  6   /* reserved */

#define REG_NAMEMAX   64    /* a key or value name, with its terminator */
#define REG_STRINGMAX 256   /* a string value, with its terminator */
#define REG_PATHMAX   512
```

A string longer than `REG_STRINGMAX - 1` is cut short when it is written.

---

## Reading

Each answers `TRUE` when the value is there and has the type asked for, and
leaves the output untouched otherwise. So the usual form seeds the output with
its default and reads over it:

```c
WORD32 Size = 12;                       /* the default */
UserReg_QueryDword(REGHIVE_LOCALCONFIGURATION, "Applications/MyApp",
                   "FontSize", &Size);
```

### UserReg_QueryString

```c
BOOL UserReg_QueryString(WORD32 Hive, PSTR Path, PSTR Name, PSTR Buffer,
                         WORD32 BufferSize);
```

user.dll · export `UserReg_QueryString1` · SDK 10

A string value, copied into `Buffer` and terminated, cut short to fit.

### UserReg_QueryQword

```c
BOOL UserReg_QueryQword(WORD32 Hive, PSTR Path, PSTR Name, WORD64* Out);
```

user.dll · export `UserReg_QueryQword1` · SDK 10

A 64-bit value.

### UserReg_QueryDword

```c
BOOL UserReg_QueryDword(WORD32 Hive, PSTR Path, PSTR Name, WORD32* Out);
```

user.dll · export `UserReg_QueryDword1` · SDK 10

A 32-bit value.

### UserReg_QueryWord

```c
BOOL UserReg_QueryWord(WORD32 Hive, PSTR Path, PSTR Name, WORD16* Out);
```

user.dll · export `UserReg_QueryWord1` · SDK 10

A 16-bit value.

### UserReg_QueryBoolean

```c
BOOL UserReg_QueryBoolean(WORD32 Hive, PSTR Path, PSTR Name, WORD8* Out);
```

user.dll · export `UserReg_QueryBoolean1` · SDK 10

A boolean, as a byte that is zero or not.

---

## Writing

Each writes one value, making the key and every key above it if they are
missing, and replacing a value of another type under the same name.
**Returns** `TRUE` when the value is now what was asked for; `FALSE` with
`ERROR_ACCESS_DENIED` for a hive the caller may not write.

### UserReg_SetString

```c
BOOL UserReg_SetString(WORD32 Hive, PSTR Path, PSTR Name, PSTR Value);
```

user.dll · export `UserReg_SetString1` · SDK 10

### UserReg_SetQword

```c
BOOL UserReg_SetQword(WORD32 Hive, PSTR Path, PSTR Name, WORD64 Value);
```

user.dll · export `UserReg_SetQword1` · SDK 10

### UserReg_SetDword

```c
BOOL UserReg_SetDword(WORD32 Hive, PSTR Path, PSTR Name, WORD32 Value);
```

user.dll · export `UserReg_SetDword1` · SDK 10

### UserReg_SetWord

```c
BOOL UserReg_SetWord(WORD32 Hive, PSTR Path, PSTR Name, WORD16 Value);
```

user.dll · export `UserReg_SetWord1` · SDK 10

### UserReg_SetBoolean

```c
BOOL UserReg_SetBoolean(WORD32 Hive, PSTR Path, PSTR Name, WORD8 Value);
```

user.dll · export `UserReg_SetBoolean1` · SDK 10

---

## Keys

### UserReg_CreateKey

```c
BOOL UserReg_CreateKey(WORD32 Hive, PSTR Path);
```

user.dll · export `UserReg_CreateKey1` · SDK 10

Makes a key and every key on the way to it. `TRUE` if it is there afterwards,
including when it already was.

### UserReg_KeyExists

```c
BOOL UserReg_KeyExists(WORD32 Hive, PSTR Path);
```

user.dll · export `UserReg_KeyExists1` · SDK 10

Whether a key is there, without making it.

### UserReg_DeleteKey

```c
BOOL UserReg_DeleteKey(WORD32 Hive, PSTR Path);
```

user.dll · export `UserReg_DeleteKey1` · SDK 10

Removes a key and everything under it. A hive's root cannot be deleted.

### UserReg_DeleteValue

```c
BOOL UserReg_DeleteValue(WORD32 Hive, PSTR Path, PSTR Name);
```

user.dll · export `UserReg_DeleteValue1` · SDK 10

Removes one value from a key.

### UserReg_RenameKey

```c
BOOL UserReg_RenameKey(WORD32 Hive, PSTR Path, PSTR NewName);
```

user.dll · export `UserReg_RenameKey1` · SDK 10

Gives the key at `Path` a new last component, keeping everything under it and
its place among its siblings. `NewName` is a name, not a path. **Returns**
`FALSE` when a sibling already has the name.

### UserReg_RenameValue

```c
BOOL UserReg_RenameValue(WORD32 Hive, PSTR Path, PSTR Name, PSTR NewName);
```

user.dll · export `UserReg_RenameValue1` · SDK 10

Renames one value in place. `FALSE` when the key already has a value called
`NewName`.

---

## Walking

Enumeration is by position rather than by a cursor, so nothing is opened or
closed and stopping early leaves nothing behind.

### UserReg_ChildCount

```c
WORD32 UserReg_ChildCount(WORD32 Hive, PSTR Path);
```

user.dll · export `UserReg_ChildCount1` · SDK 10

How many keys and values a key holds together, or 0 when it is not there.

### UserReg_Enumerate

```c
BOOL UserReg_Enumerate(WORD32 Hive, PSTR Path, WORD32 Index, PSTR NameBuffer,
                       WORD32 NameBufferSize, WORD32* Type);
```

user.dll · export `UserReg_Enumerate1` · SDK 10

One child of a key by position: its name, and in `*Type` what it is —
`REGTYPE_KEY` for a key, otherwise the value's type. **Returns** `FALSE` past
the last child.

```c
char Name[REG_NAMEMAX];
WORD32 Type;

for (WORD32 i = 0; UserReg_Enumerate(REGHIVE_LOCALCONFIGURATION,
        "Applications/MyApp", i, Name, sizeof(Name), &Type); i++)
    ...;
```

### UserReg_Flush

```c
VOID UserReg_Flush(VOID);
```

user.dll · export `UserReg_Flush1` · SDK 10

Writes both hives out now rather than on the next tick. Wanted before
something that will not come back, and not otherwise.
