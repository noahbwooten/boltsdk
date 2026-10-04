# The C library subset

`#include <string.h>`, `<stdio.h>`, `<math.h>`, `<stdarg.h>`, `<stddef.h>` ·
exported by **user.dll** · link **`user.lib`**

BoltOS has no C runtime: nothing initialises one in an application, and the
SDK compiles with no system headers at all. What it has instead is this
subset, under the names the language gives them, exported by user.dll.

These are not the Bolt API. They are not numbered and not versioned — an
application calls `memcpy`, and the compiler itself emits calls to `memcpy`
and `memset` for structure copies and clears, by name. Their names and
signatures are the standard's and stay that way.

What follows is what is there, and where it is not what a C library would do.

## string.h

```c
void*  memcpy(void* dest, const void* src, size_t n);
void*  memmove(void* dest, const void* src, size_t n);
void*  memset(void* s, int c, size_t n);
int    memcmp(const void* s1, const void* s2, size_t n);
void*  memchr(const void* ptr, int ch, size_t count);

char*  strcpy(char* dest, const char* src);
char*  strncpy(char* dest, const char* src, size_t n);
size_t strlcpy(char* dest, const char* src, size_t size);
char*  strcat(char* dest, const char* src);
size_t strlcat(char* dest, const char* src, size_t size);

int    strcmp(const char* s1, const char* s2);
int    strncmp(const char* s1, const char* s2, size_t n);
int    strcoll(const char* s1, const char* s2);
size_t strxfrm(char* dest, const char* src, size_t n);

size_t strlen(const char* s);
char*  strchr(const char* s, int c);
char*  strrchr(const char* s, int c);
char*  strstr(const char* haystack, const char* needle);
char*  strpbrk(const char* s, const char* accept);
size_t strspn(const char* s, const char* accept);
size_t strcspn(const char* s, const char* reject);
char*  strtok(char* s, const char* delim);
char*  strtok_r(char* s, const char* delim, char** saveptr);

char*  strdup(char* s);
```

- `strlcpy` and `strlcat` are the BSD forms: they always terminate, and they are
  the ones to use for a fixed buffer.
- `strdup` takes `char*`, not `const char*`, and its copy is allocated with
  [`User_Allocate`](user.md#user_allocate): free it with `User_Free`.
- `strcoll` and `strxfrm` have no locale; they compare and copy bytes.
- `strncat` and the wide-character functions are not provided. `wchar.h`
  declares `wchar_t` (16 bits) and nothing else.

## stdio.h

```c
int  sprintf(char* buffer, const char* format, ...);
int  vsprintf(char* buffer, const char* format, va_list args);
int  sscanf(const char* str, const char* format, ...);
int  vsscanf(const char* str, const char* format, va_list args);
void qsort(void* base, unsigned long num, unsigned long size,
           int (*cmp)(const void*, const void*));
```

There are no files, streams or consoles here: [user.h](user.md#files) has the
file calls. There is no `snprintf`, so size the buffer for the longest result.

`sprintf` understands:

| | |
|---|---|
| `%d`, `%i` | `int`; `%02d` pads to two digits, the only width there is |
| `%u` | `unsigned int` |
| `%x` | `unsigned int`, in lower-case hex, no prefix |
| `%lld`, `%lli`, `%llu`, `%llx` | 64-bit |
| `%s` | a string |
| `%p` | a pointer, in hex |
| `%%` | a percent sign |

Two things differ from a C library and matter:

- **`l` means 64 bits.** `%ld` takes a `long long`. `WORD32` is a 32-bit
  `unsigned long`, so print one with `%u`, and a `WORD64` with `%llu`.
- **No `%c`, no `%f`, no precision.** `%.10s` takes the next argument as the
  string. Cut a string to length with `strlcpy` into a buffer first.

`sscanf` understands `%d`, `%u`, `%x` (hex digits, no prefix) and `%c`; a space
in the format skips spaces in the input, and any other character must match.
It answers how many conversions it made.

`qsort` sorts in place, as the standard's does.

## math.h

```c
double sqrt(double x);
float  sqrtf(float x);
double sin(double x);     /* _sin underneath */
double cos(double x);     /* _cos underneath */
```

Angles are in radians. That is all of it in SDK 10.

## stdarg.h and stddef.h

`va_list`, `va_start`, `va_arg` and `va_end`, for the x64 calling convention;
`size_t` and `ptrdiff_t` (64 bits), and `NULL`.
