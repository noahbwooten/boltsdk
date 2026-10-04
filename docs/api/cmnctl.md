# cmnctl.h — the immediate-mode controls

`#include <cmnctl.h>` · exported by **cmnctl.dll** · link **`cmnctl.lib`**

The original controls, which are drawn rather than kept: each call draws the
control and answers what happened to it this frame, and the application holds
the state between frames. They belong to the original window contract,
`__WindowProcedure`, which is called every frame in screen coordinates; see
the [application model](../guide/application-model.md#windows).

A new application uses the [retained controls](cmnctl2.md) instead, which
paint only when something changes and handle the keyboard and Tab properly.
These are frozen as they are, for the applications written against them, and
the two libraries share no code.

```c
__declspec(dllexport) void __WindowProcedure(WORD32 X, WORD32 Y, WORD32 W,
    WORD32 H, WORD32 MouseX, WORD32 MouseY, BOOL LeftButton, WORD64 Reserved)
{
    static BOOL Ticked;

    if (CmnCtl_DrawButton(X + 10, Y + 40, 80, 30, "Go"))
        ...;   /* pressed this frame */

    CmnCtl_DrawCheckbox(X + 10, Y + 80, "Ticked", &Ticked);
}
```

The pointer the controls answer to is the one the window manager handed the
window being drawn, so they work only inside a window procedure. A control
that acts on a click consumes it, so the press does not also land on whatever
is drawn after it.

---

### CmnCtl_DrawButton

```c
BOOL CmnCtl_DrawButton(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, char* Text);
```

cmnctl.dll · export `CmnCtl_DrawButton1` · SDK 10

A button, `W` by `H`, with its caption centred. **Returns** `TRUE` on the
frame it is clicked.

### CmnCtl_DrawCheckbox

```c
BOOL CmnCtl_DrawCheckbox(WORD32 X, WORD32 Y, PSTR Title, PBOOL State);
```

cmnctl.dll · export `CmnCtl_DrawCheckbox1` · SDK 10

A 20-pixel box with `Title` beside it, filled while `*State` is 1. A click
turns `*State` over. **Returns** `TRUE` on the frame it changed.

### CmnCtl_DrawComboBox

```c
BOOL CmnCtl_DrawComboBox(WORD32 X, WORD32 Y, WORD32 W, PSTR Title,
                         WORD32 ElementCnt, PSTR* Elements, PWORD32 Selected,
                         PBOOL IsOpen);
```

cmnctl.dll · export `CmnCtl_DrawComboBox1` · SDK 10

`Title`, then a box `W` wide showing `Elements[*Selected]`. A click opens it
(`*IsOpen` becomes 1) and draws the list below; a click on an item sets
`*Selected` and closes it. **Returns** `TRUE` on the frame an item is chosen.

### CmnCtl_DrawTextInput

```c
VOID CmnCtl_DrawTextInput(WORD32 X, WORD32 Y, WORD32 W, PSTR Text,
                          PBOOL IsOpen, PWORD32 CursorPos);
```

cmnctl.dll · export `CmnCtl_DrawTextInput1` · SDK 10

A one-line box `W` wide showing `Text`, which takes typing while the pointer is
over it, editing `Text` in place at `*CursorPos`. `Text` must have room for
`W / 8` characters and a terminator. `IsOpen` is reserved.

### CmnCtl_DrawTextInputML

```c
VOID CmnCtl_DrawTextInputML(WORD32 X, WORD32 Y, WORD32 W, WORD32 Lines,
                            PSTR Text, PWORD32 CursorX, PWORD32 CursorY,
                            PBOOL IsInFocus, PSTR** RichCtx);
```

cmnctl.dll · export `CmnCtl_DrawTextInputML1` · SDK 10

A box of `Lines` lines, `W` wide, editing `Text` in place. `Text` must have
room for `(W / 8) * Lines` characters and a terminator. `*CursorX` and
`*CursorY` are the caret; `*IsInFocus` whether it takes keys; `*RichCtx` is a
line table the control keeps between frames, which starts `NULL`.

### CmnCtl_ClockTextInputML

```c
VOID CmnCtl_ClockTextInputML(PINPUT_ML Input);
```

cmnctl.dll · export `CmnCtl_ClockTextInputML1` · SDK 10

Rebuilds an `INPUT_ML`'s line table from its text — `Text` broken at newlines
and every `MAX_LINE_LENGTH` characters into `Lines` rows — freeing the old
table.

```c
typedef struct _INPUT_ML {
    char** RichText, * Text;
    int Lines;
    int MAX_TEXT_LENGTH;
    int MAX_LINE_LENGTH;
} INPUT_ML, *PINPUT_ML;
```

### CmnCtl_DrawIntInput

```c
BOOL CmnCtl_DrawIntInput(WORD32 X, WORD32 Y, WORD32 W, INT Min, INT Max,
                         PINT Value);
```

cmnctl.dll · export `CmnCtl_DrawIntInput1` · SDK 10

`*Value` in a box `W` wide, with + and − buttons at its right that step it
within `Min` and `Max`. **Returns** `TRUE` on the frame it changed.

### CmnCtl_CreateTabController

```c
PTAB_CTL CmnCtl_CreateTabController(WORD32 TabCount, PSTR* TabNames);
```

cmnctl.dll · export `CmnCtl_CreateTabController1` · SDK 10

A tab bar's state: the names are copied, the first tab is selected. Free it
with [`CmnCtl_DestroyTabController`](#cmnctl_destroytabcontroller).

```c
typedef struct _TAB_CTL {
    WORD32 TabCount;
    WORD32 SelectedTab;
    WORD32* TabSizes;
    PSTR* TabNames;
} TAB_CTL, *PTAB_CTL;
```

### CmnCtl_DestroyTabController

```c
VOID CmnCtl_DestroyTabController(PTAB_CTL TabCtl);
```

cmnctl.dll · export `CmnCtl_DestroyTabController1` · SDK 10

### CmnCtl_DrawTabController

```c
VOID CmnCtl_DrawTabController(PTAB_CTL TabCtl, WORD32 X, WORD32 Y, WORD32 W);
```

cmnctl.dll · export `CmnCtl_DrawTabController1` · SDK 10

Draws the bar, 30 pixels tall and `W` wide; a click on a tab selects it.

### CmnCtl_DrawTab

```c
BOOL CmnCtl_DrawTab(PTAB_CTL TabCtl, WORD32 TabId);
```

cmnctl.dll · export `CmnCtl_DrawTab1` · SDK 10

Whether tab `TabId` is the selected one, so a page is drawn only when its tab
is:

```c
CmnCtl_DrawTabController(Tabs, X + 10, Y + 40, W - 20);
if (CmnCtl_DrawTab(Tabs, 0)) { /* the first page */ }
if (CmnCtl_DrawTab(Tabs, 1)) { /* the second */ }
CmnCtl_EndTabHandler(Tabs);
```

### CmnCtl_EndTabHandler

```c
VOID CmnCtl_EndTabHandler(PTAB_CTL TabCtl);
```

cmnctl.dll · export `CmnCtl_EndTabHandler1` · SDK 10

Closes the pages after the last `CmnCtl_DrawTab`. Does nothing in SDK 10, and
is called for the sake of applications that rely on the pairing.

### CmnCtl_DrawDropMenu

```c
VOID CmnCtl_DrawDropMenu(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon,
                         PBOOL State, PVOID StateChangedCb);
```

cmnctl.dll · export `CmnCtl_DrawDropMenu1` · SDK 10

A heading `W` wide, with an icon and an arrow, that opens and closes a group
of items beneath it: a click turns `*State` over. `StateChangedCb`, when not
`NULL`, is called as `void Callback(void)` when it does.

### CmnCtl_DrawDropItem

```c
VOID CmnCtl_DrawDropItem(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon);
```

cmnctl.dll · export `CmnCtl_DrawDropItem1` · SDK 10

One row beneath an open drop menu: an icon and a name.

### CmnCtl_DrawDropItemSelectable

```c
BOOL CmnCtl_DrawDropItemSelectable(WORD32 X, WORD32 Y, WORD32 W, PSTR Name,
                                   PVOID Icon, BOOL Selected);
```

cmnctl.dll · export `CmnCtl_DrawDropItemSelectable1` · SDK 10

The same row, highlighted when `Selected` and answering clicks. **Returns**
`TRUE` on the frame it is clicked.

### CmnCtl_DrawMeter

```c
VOID CmnCtl_DrawMeter(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Value,
                      WORD32 Max, PSTR Suffix);
```

cmnctl.dll · export `CmnCtl_DrawMeter1` · SDK 10

A value now: a segmented column filled to `Value` out of `Max`, with the
reading and `Suffix` ("%", "MB") along its bottom. `NULL` for no readout and no
room kept for one.

### CmnCtl_CreateGraph

```c
PGRAPH_CTL CmnCtl_CreateGraph(WORD32 SampleCount, WORD32 Max);
```

cmnctl.dll · export `CmnCtl_CreateGraph1` · SDK 10

The same value over time: a ring of `SampleCount` readings. `Max` is full
scale, or zero to scale to the largest reading held. The period it covers is
`SampleCount` times however often it is pushed, so push from a timer, not from
painting.

### CmnCtl_DestroyGraph

```c
VOID CmnCtl_DestroyGraph(PGRAPH_CTL Graph);
```

cmnctl.dll · export `CmnCtl_DestroyGraph1` · SDK 10

### CmnCtl_PushGraphSample

```c
VOID CmnCtl_PushGraphSample(PGRAPH_CTL Graph, WORD32 Value);
```

cmnctl.dll · export `CmnCtl_PushGraphSample1` · SDK 10

Adds a reading, dropping the oldest once the ring is full.

### CmnCtl_DrawGraph

```c
VOID CmnCtl_DrawGraph(PGRAPH_CTL Graph, WORD32 X, WORD32 Y, WORD32 W,
                      WORD32 H);
```

cmnctl.dll · export `CmnCtl_DrawGraph1` · SDK 10

Draws the readings as a trace over a grid, newest at the right.

### CmnCtl_GetColors

```c
PWNDMGR_COLORS CmnCtl_GetColors(void);
```

cmnctl.dll · export `CmnCtl_GetColors1` · SDK 10

The palette, the same table the window manager's
[`Wndmgr_GetColors`](wndmgr.md#wndmgr_getcolors) answers.
