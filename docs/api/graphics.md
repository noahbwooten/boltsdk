# graphics.h — drawing

`#include <graphics.h>` · exported by **user.dll** · link **`user.lib`**

The primitives: rectangles, text, circles, and blocks of pixels, all drawn
into whatever the system is pointing them at. During a message-driven
window's `MSG_PAINT` that is the window's own surface, in window coordinates —
(0, 0) is the window's top-left corner, the title bar takes the first
`WNDMGR_TITLEH` (30) rows, and the client area is below it. Drawing at any
other time draws nowhere useful; ask for a repaint with
[`Wndmgr_InvalidateWindow`](wndmgr.md#wndmgr_invalidatewindow) instead. An
application on the original immediate-mode contract is handed screen
coordinates and draws every frame; see the
[application model](../guide/application-model.md#windows).

Most applications draw with the retained controls in
[cmnctl2.h](cmnctl2.md) and reach for these only for something of their own.

## Colours and pixels

A colour is a `WORD32` holding `0x00RRGGBB`, which [`Ge_Color`](#ge_color)
builds. The window manager's palette ([`Wndmgr_GetColors`](wndmgr.md#wndmgr_getcolors))
holds the colours the rest of the system draws with, and using those keeps an
application looking like it belongs.

A block of pixels — what [`Ge_DrawBuffer`](#ge_drawbuffer) draws and what
[`UserRes_LoadImage`](modres.md#userres_loadimage) and
[`Wndmgr_LoadIcon`](wndmgr.md#wndmgr_loadicon) hand back — is 24-bit: three
bytes a pixel, in the order blue, green, red, rows from the top, with no
padding between rows. Magenta (255, 0, 255) is transparent: a pixel of exactly
that colour is not drawn.

## Fonts

```c
#define GEFONT_NORMAL  0   /* 8 pixels a character */
#define GEFONT_BOLD    1   /* the same, struck twice a pixel apart: 9 */
#define GEFONT_OUTLINE 2   /* with a dark edge around it: 8, plus 2 overall */
```

One typeface, fixed width, 15 pixels tall. Text is drawn as one line; a
newline is not a line break.

---

### Ge_Color

```c
WORD32 Ge_Color(BYTE r, BYTE g, BYTE b, BYTE a);
```

user.dll · export `Ge_Color1` · SDK 10

A colour from its red, green and blue. Alpha is accepted and ignored: nothing
in SDK 10 blends.

### Ge_DrawFilled

```c
VOID Ge_DrawFilled(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Color);
```

user.dll · export `Ge_DrawFilled1` · SDK 10

A solid rectangle, `W` by `H`, with its top-left corner at (`X`, `Y`).

### Ge_DrawOutline

```c
VOID Ge_DrawOutline(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Color);
```

user.dll · export `Ge_DrawOutline1` · SDK 10

The edge of the same rectangle, one pixel wide, drawn inside its bounds.

### Ge_DrawText

```c
VOID Ge_DrawText(WORD32 X, WORD32 Y, PSTR Text, WORD32 Color, WORD8 Font);
```

user.dll · export `Ge_DrawText1` · SDK 10

One line of text with the top-left of its first character at (`X`, `Y`).
`Font` is a `GEFONT_*`. A `NULL` string draws nothing.

### Ge_GetTextLength

```c
VOID Ge_GetTextLength(PSTR Text, PWORD32 OutLength, WORD32 Font);
```

user.dll · export `Ge_GetTextLength1` · SDK 10

How wide a string draws in a font, in pixels, written to `*OutLength`. For
centring and right-aligning: `X + (W - Length) / 2`.

### Ge_DrawCircleFilled

```c
VOID Ge_DrawCircleFilled(WORD32 X, WORD32 Y, WORD32 R, WORD32 Color);
```

user.dll · export `Ge_DrawCircleFilled1` · SDK 10

A solid circle of radius `R` centred on (`X`, `Y`).

### Ge_DrawCircleOutline

```c
VOID Ge_DrawCircleOutline(WORD32 X, WORD32 Y, WORD32 R, WORD32 Color);
```

user.dll · export `Ge_DrawCircleOutline1` · SDK 10

The edge of the same circle, one pixel wide.

### Ge_DrawBuffer

```c
VOID Ge_DrawBuffer(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, PVOID Data);
```

user.dll · export `Ge_DrawBuffer1` · SDK 10

A block of 24-bit pixels, `W` by `H`, laid out as [above](#colours-and-pixels).
Magenta pixels are skipped, which is how an icon has a shape. `NULL` draws
nothing.

### Ge_SetClip

```c
VOID Ge_SetClip(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
```

user.dll · export `Ge_SetClip1` · SDK 10

Confines every primitive to a rectangle until it is cleared, so a list can
draw rows past its own edge and have them cut off rather than counted out
beforehand. A zero width or height means no clip. A control that narrows the
clip puts back what it found with [`Ge_GetClip`](#ge_getclip) rather than
clearing it, so it does not expose whatever contains it.

### Ge_GetClip

```c
VOID Ge_GetClip(PWORD32 X, PWORD32 Y, PWORD32 W, PWORD32 H);
```

user.dll · export `Ge_GetClip1` · SDK 10

The clip in force, all zeros for none. Any pointer may be `NULL`.

### Ge_ClearClip

```c
VOID Ge_ClearClip(VOID);
```

user.dll · export `Ge_ClearClip1` · SDK 10

Removes the clip.

### Ge_StretchBitmap

```c
VOID Ge_StretchBitmap(PVOID Bits, WORD32 OldWidth, WORD32 OldHeight,
                      WORD32 NewWidth, WORD32 NewHeight, PVOID OutBits);
```

user.dll · export `Ge_StretchBitmap1` · SDK 10

Scales a block of 24-bit pixels to another size by taking the nearest pixel,
writing into `OutBits`, which the caller provides.

In SDK 10 this is for square images only: rows are counted with `NewWidth`, so
`NewWidth` and `NewHeight` must be equal and `OutBits` must hold
`NewWidth * NewWidth * 3` bytes. For icons, which are square, ask
[`Wndmgr_LoadIcon`](wndmgr.md#wndmgr_loadicon) for the size wanted instead.

### Ge_GetsBitForBmp

```c
VOID Ge_GetsBitForBmp(PVOID Bmp, PVOID* Bits, WORD32* BitSize);
```

user.dll · export `Ge_GetsBitForBmp1` · SDK 10

The pixel data out of a `.bmp` file held in memory, copied into a block the
caller owns and frees with [`User_Free`](user.md#user_free). Only an
uncompressed 24-bit bitmap is taken; for anything else `*Bits` is `NULL` and
`*BitSize` zero.

The pixels are exactly as the file stores them — rows from the bottom up, each
padded to a multiple of four bytes — which is not the layout
[`Ge_DrawBuffer`](#ge_drawbuffer) draws; the width and height are in the
file's header. A picture the application draws is better carried as a
resource and loaded with [`UserRes_LoadImage`](modres.md#userres_loadimage),
which hands back pixels ready to draw.
