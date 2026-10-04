# cmnctl2.h — the retained controls

`#include <cmnctl2.h>` · exported by **cmnctl2.dll** · link **`cmnctl2.lib`**

Controls that exist, rather than controls that are drawn: a control is made
once, owns its text, position and state, and says when it has changed, so a
window with nothing new to show is not painted. This is what a new
application builds its window from. The calculator example is built on it.

```c
case MSG_INIT:
    Save = CmnCtl2_CreateButton(Window, 10, 40, 80, 30, "Save");
    return 0;

case MSG_PAINT:
    CmnCtl2_Paint(Window);
    return 0;

case MSG_QUIT:
    CmnCtl2_DestroyControls(Window);
    return 0;

default:
    if (CmnCtl2_Dispatch(Window, Message, Param1, Param2))
        return 1;
    if (CmnCtl2_WasClicked(Save))
        ...;
    return 0;
```

Every control is a `HANDLE` owned by its window. Coordinates are
window-relative — (0, 0) is the window's corner, the title bar is the first
`WNDMGR_TITLEH` (30) rows — and stay put when the window is dragged. Controls
are painted, hit-tested and reached with Tab in the order they were made.

Every setter marks the window for repainting, so an application using these
never has to invalidate anything, and setting a value to what it already was
marks nothing. Create calls answer **0** when they fail.

## Kinds and flags

```c
#define CTL2_LABEL     1      #define CTL2_TABBAR    7
#define CTL2_BUTTON    2      #define CTL2_METER     8
#define CTL2_CHECKBOX  3      #define CTL2_GRAPH     9
#define CTL2_TEXTINPUT 4      #define CTL2_TEXTAREA  10
#define CTL2_INTINPUT  5      #define CTL2_TABLE     11
#define CTL2_COMBOBOX  6      #define CTL2_TREE      12
                              #define CTL2_PANEL     13
                              #define CTL2_SCROLLBAR 14

#define CTL2F_DISABLED     0x0001  /* drawn dimmed, ignores input */
#define CTL2F_HIDDEN       0x0002  /* not drawn, ignores input */
#define CTL2F_NOFOCUS      0x0004  /* skipped by Tab */
#define CTL2F_PANELHIDDEN  0x0008  /* hidden with its panel */
#define CTL2F_NOBACKGROUND 0x0010  /* draws its text and nothing under it */
```

---

## The framework

### CmnCtl2_Paint

```c
VOID CmnCtl2_Paint(HANDLE Window);
```

cmnctl2.dll · export `CmnCtl2_Paint1` · SDK 10

Draws every control on a window. Call it from `MSG_PAINT` and nowhere else.

### CmnCtl2_Dispatch

```c
WORD64 CmnCtl2_Dispatch(HANDLE Window, WORD32 Message, WORD64 Param1,
                        WORD64 Param2);
```

cmnctl2.dll · export `CmnCtl2_Dispatch1` · SDK 10

Gives a message to the controls first: the pointer, the keys, the wheel,
Tab between controls. **Returns** non-zero when a control used it, so an
application can skip its own handling — a keystroke that went into a text
box should not also be a shortcut. Pass every message the window procedure
does not handle itself.

### CmnCtl2_WasClicked

```c
BOOL CmnCtl2_WasClicked(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_WasClicked1` · SDK 10

Whether a control was activated since anybody last asked: a button pressed, a
checkbox ticked, a combo box or tab bar changed, a row picked. Reads and
clears, so it is safe to ask after every dispatch.

### CmnCtl2_WasDoubleClicked

```c
BOOL CmnCtl2_WasDoubleClicked(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_WasDoubleClicked1` · SDK 10

Whether that click was the second of a pair, on the same control in the same
place. Reported beside the ordinary click, not instead of it: ask both.

### CmnCtl2_GetColors

```c
PVOID CmnCtl2_GetColors(VOID);
```

cmnctl2.dll · export `CmnCtl2_GetColors1` · SDK 10

The palette the controls draw with, which is the window manager's
`WNDMGR_COLORS`; cast it. See [`Wndmgr_GetColors`](wndmgr.md#wndmgr_getcolors).

---

## Windows

### CmnCtl2_CreateWindow

```c
HANDLE CmnCtl2_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H,
                            PSTR Title, PVOID Icon);
```

cmnctl2.dll · export `CmnCtl2_CreateWindow1` · SDK 10

A secondary window — a dialogue, a palette — with no taskbar entry. It is a
window manager window in every other way, driven by the application's
`__WindowProcedure2` like the rest; see
[`Wndmgr_CreateWindow`](wndmgr.md#wndmgr_createwindow), which makes an
application's main window. `Title` and `Icon` are kept, not copied.

### CmnCtl2_DestroyWindow

```c
VOID CmnCtl2_DestroyWindow(HANDLE Window);
```

cmnctl2.dll · export `CmnCtl2_DestroyWindow1` · SDK 10

Destroys every control on a window, then closes it.

### CmnCtl2_DestroyControls

```c
VOID CmnCtl2_DestroyControls(HANDLE Window);
```

cmnctl2.dll · export `CmnCtl2_DestroyControls1` · SDK 10

Takes down every control on a window and leaves the window. Call it from
`MSG_QUIT`: the window manager closes the window once that returns, and
without this the controls outlive it until the application ends.

### CmnCtl2_DestroyControl

```c
VOID CmnCtl2_DestroyControl(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_DestroyControl1` · SDK 10

Destroys one control. Given a panel, it takes the panel's members too.

---

## Making controls

Each takes the window, or a panel on it, that the control goes on.

### CmnCtl2_CreateLabel

```c
HANDLE CmnCtl2_CreateLabel(HANDLE Window, WORD32 X, WORD32 Y, PSTR Text);
```

cmnctl2.dll · export `CmnCtl2_CreateLabel1` · SDK 10

A line of text. Copied; change it with [`CmnCtl2_SetText`](#cmnctl2_settext).
Never takes the focus.

### CmnCtl2_CreateButton

```c
HANDLE CmnCtl2_CreateButton(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                            WORD32 H, PSTR Text);
```

cmnctl2.dll · export `CmnCtl2_CreateButton1` · SDK 10

A push button, `W` by `H`, with its caption centred. Pressed by a click, or
by Space while it has the focus; ask
[`CmnCtl2_WasClicked`](#cmnctl2_wasclicked).

### CmnCtl2_CreateCheckbox

```c
HANDLE CmnCtl2_CreateCheckbox(HANDLE Window, WORD32 X, WORD32 Y, PSTR Text,
                              BOOL Checked);
```

cmnctl2.dll · export `CmnCtl2_CreateCheckbox1` · SDK 10

A box with a caption beside it. Its value is 1 when ticked.

### CmnCtl2_CreateTextInput

```c
HANDLE CmnCtl2_CreateTextInput(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                               WORD32 Capacity);
```

cmnctl2.dll · export `CmnCtl2_CreateTextInput1` · SDK 10

A one-line text box, `W` wide. `Capacity` is how much text it holds, its
terminator included; the control owns the buffer.

### CmnCtl2_CreateTextArea

```c
HANDLE CmnCtl2_CreateTextArea(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                              WORD32 Lines, WORD32 Capacity);
```

cmnctl2.dll · export `CmnCtl2_CreateTextArea1` · SDK 10

A multi-line text box showing `Lines` rows, wrapping at its width and
scrolling when it holds more. `Capacity` bytes, owned by the control.

### CmnCtl2_CreateIntInput

```c
HANDLE CmnCtl2_CreateIntInput(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                              INT Min, INT Max, INT Value);
```

cmnctl2.dll · export `CmnCtl2_CreateIntInput1` · SDK 10

A number between `Min` and `Max`, with buttons to step it, starting at
`Value`.

### CmnCtl2_CreateComboBox

```c
HANDLE CmnCtl2_CreateComboBox(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                              PSTR Title, WORD32 Count, PSTR* Items);
```

cmnctl2.dll · export `CmnCtl2_CreateComboBox1` · SDK 10

A drop-down choice of `Count` items, with a caption. The items are not copied
and must live as long as the control: string literals, or a table the
application keeps. The value is the index chosen. The open list may hang past
the window's edge.

### CmnCtl2_CreateTabBar

```c
HANDLE CmnCtl2_CreateTabBar(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                            WORD32 Count, PSTR* Names);
```

cmnctl2.dll · export `CmnCtl2_CreateTabBar1` · SDK 10

A row of tabs. The names are not copied. The value is the tab selected; pair
it with a [panel](#panels) per page and show the one that matches. Tabs that do
not fit scroll.

### CmnCtl2_CreateMeter

```c
HANDLE CmnCtl2_CreateMeter(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                           WORD32 H);
```

cmnctl2.dll · export `CmnCtl2_CreateMeter1` · SDK 10

A segmented bar showing a reading out of 100, set with
[`CmnCtl2_SetValue`](#cmnctl2_setvalue).

### CmnCtl2_CreateGraph

```c
HANDLE CmnCtl2_CreateGraph(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                           WORD32 H, WORD32 Samples, WORD32 Max);
```

cmnctl2.dll · export `CmnCtl2_CreateGraph1` · SDK 10

A reading over time, holding `Samples` readings and scrolling as new ones are
pushed with [`CmnCtl2_PushSample`](#cmnctl2_pushsample). `Max` is the full
scale, or zero to scale to the largest reading held.

---

## Panels

A group of controls shown, hidden and destroyed together, placing what is put
on it against its own corner. Every create call takes a panel wherever it
takes a window:

```c
HANDLE Page = CmnCtl2_CreatePanel(Window, 15, 82, 600, 400);
CmnCtl2_CreateButton(Page, 5, 10, 80, 24, "Apply");   /* at 20, 92 */
CmnCtl2_SetHidden(Page, TRUE);                        /* the button too */
```

A panel is a group, not a container: it draws nothing, cannot be clicked, and
clips nothing, and its members are painted and reached by Tab exactly as they
would be without it. Its position is fixed when it is made.

### CmnCtl2_CreatePanel

```c
HANDLE CmnCtl2_CreatePanel(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                           WORD32 H);
```

cmnctl2.dll · export `CmnCtl2_CreatePanel1` · SDK 10

A panel at (`X`, `Y`), `W` by `H`.

---

## Reading and changing controls

### CmnCtl2_SetText

```c
VOID CmnCtl2_SetText(HANDLE Control, PSTR Text);
```

cmnctl2.dll · export `CmnCtl2_SetText1` · SDK 10

A control's text: a label's words, a button's caption, a text box's contents.
Copied.

### CmnCtl2_GetText

```c
PSTR CmnCtl2_GetText(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_GetText1` · SDK 10

A control's text, as the control's own storage: read it, copy it, do not keep
it past the control. A text box with nothing typed answers its placeholder.

### CmnCtl2_SetPlaceholder

```c
VOID CmnCtl2_SetPlaceholder(HANDLE Control, PSTR Text);
```

cmnctl2.dll · export `CmnCtl2_SetPlaceholder1` · SDK 10

What a text box shows, dimmed, while nothing has been typed — and what
[`CmnCtl2_GetText`](#cmnctl2_gettext) answers then. A default value, not a
hint, so a caller never tests for empty.

### CmnCtl2_SetValue

```c
VOID CmnCtl2_SetValue(HANDLE Control, WORD32 Value);
```

cmnctl2.dll · export `CmnCtl2_SetValue1` · SDK 10

A control's number, whose meaning depends on its kind: ticked or not for a
checkbox, the number for an integer input, the selection for a combo box or
tab bar, the reading for a meter, the percentage for a graph.

### CmnCtl2_GetValue

```c
WORD32 CmnCtl2_GetValue(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_GetValue1` · SDK 10

The same number, read back.

### CmnCtl2_GetSelection

```c
WORD32 CmnCtl2_GetSelection(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_GetSelection1` · SDK 10

Which tab, item or row is selected. The same as
[`CmnCtl2_GetValue`](#cmnctl2_getvalue), except on a table, where it is the
row holding the selected key.

### CmnCtl2_SetEnabled

```c
VOID CmnCtl2_SetEnabled(HANDLE Control, BOOL Enabled);
```

cmnctl2.dll · export `CmnCtl2_SetEnabled1` · SDK 10

A disabled control is drawn dimmed and ignores input.

### CmnCtl2_SetHidden

```c
VOID CmnCtl2_SetHidden(HANDLE Control, BOOL Hidden);
```

cmnctl2.dll · export `CmnCtl2_SetHidden1` · SDK 10

A hidden control is not drawn and ignores input. Hiding a panel hides its
members without disturbing which of them the page had hidden itself.

### CmnCtl2_SetBackground

```c
VOID CmnCtl2_SetBackground(HANDLE Control, BOOL Painted);
```

cmnctl2.dll · export `CmnCtl2_SetBackground1` · SDK 10

Whether a control paints its own background, which it does by default. Off,
the application paints what is underneath and the control puts only its text
and caret over it — for a text box on a surface of the application's own.

### CmnCtl2_SetFocus

```c
VOID CmnCtl2_SetFocus(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_SetFocus1` · SDK 10

Gives a control the keyboard, as clicking it would, so the first keystroke
lands in it. Ignored for a control that never takes the focus.

### CmnCtl2_SetPosition

```c
VOID CmnCtl2_SetPosition(HANDLE Control, WORD32 X, WORD32 Y);
```

cmnctl2.dll · export `CmnCtl2_SetPosition1` · SDK 10

Moves a control, in the coordinates it was made in — for a control on a
panel, the panel's.

### CmnCtl2_SetSize

```c
VOID CmnCtl2_SetSize(HANDLE Control, WORD32 W, WORD32 H);
```

cmnctl2.dll · export `CmnCtl2_SetSize1` · SDK 10

Resizes a control, for a window laying itself out again after `MSG_RESIZE`. A
text area, table or tree measures its height in rows, so the height is
rounded down to a whole number of them.

### CmnCtl2_GetHeight

```c
WORD32 CmnCtl2_GetHeight(HANDLE Control);
```

cmnctl2.dll · export `CmnCtl2_GetHeight1` · SDK 10

How tall a control ended up, so something can go directly beneath it.

### CmnCtl2_PushSample

```c
VOID CmnCtl2_PushSample(HANDLE Graph, WORD32 Value);
```

cmnctl2.dll · export `CmnCtl2_PushSample1` · SDK 10

Adds a reading to a graph. Push from a timer: a graph fed once a second
repaints once a second.

---

## Scroll bars

A control that scrolls itself — a table, a tree, a text area — grows its own
bar and answers the wheel without being asked. These are for a bar placed by
hand beside something of the application's own.

```c
HANDLE Bar = CmnCtl2_CreateScrollBar(Window, 400, 60, 240);
CmnCtl2_SetScrollRange(Bar, 500, 12);    /* 500 items, 12 showing */
```

The position is the first item shown, and stops with the last item at the
bottom. A bar is `CTL2_SCROLLBAR_W` (14) pixels wide.

### CmnCtl2_CreateScrollBar

```c
HANDLE CmnCtl2_CreateScrollBar(HANDLE Window, WORD32 X, WORD32 Y, WORD32 H);
```

cmnctl2.dll · export `CmnCtl2_CreateScrollBar1` · SDK 10

A vertical bar `H` tall, with a button at each end and a thumb between.

### CmnCtl2_SetScrollRange

```c
VOID CmnCtl2_SetScrollRange(HANDLE Bar, WORD32 Total, WORD32 Visible);
```

cmnctl2.dll · export `CmnCtl2_SetScrollRange1` · SDK 10

How many items there are and how many are on screen. A bar showing
everything is drawn flat and refuses to move.

### CmnCtl2_GetScrollPos

```c
WORD32 CmnCtl2_GetScrollPos(HANDLE Bar);
```

cmnctl2.dll · export `CmnCtl2_GetScrollPos1` · SDK 10

The first item shown. Read it when painting what the bar scrolls, after
[`CmnCtl2_Dispatch`](#cmnctl2_dispatch) has had the pointer and the wheel.

### CmnCtl2_SetScrollPos

```c
VOID CmnCtl2_SetScrollPos(HANDLE Bar, WORD32 Position);
```

cmnctl2.dll · export `CmnCtl2_SetScrollPos1` · SDK 10

Moves the bar, clamped to its range.

---

## Tables

Columns declared once, rows written into storage the table owns. Cell text is
copied and compared before it is written, so a refresh that changes two
figures out of two hundred repaints because of those two.

```c
HANDLE T = CmnCtl2_CreateTable(Window, 10, 76, 600, 12, 0);
CmnCtl2_AddColumn(T, "Name", 220, CTL2ALIGN_LEFT);
CmnCtl2_AddColumn(T, "Memory", 90, CTL2ALIGN_RIGHT);

CmnCtl2_SetRowCount(T, Count);
for (WORD32 i = 0; i < Count; i++) {
    CmnCtl2_SetRowKey(T, i, Ids[i]);
    CmnCtl2_SetCell(T, i, 0, Names[i]);
}
```

A table taller than its room scrolls, answers the wheel and the page keys when
it has the focus, and keeps the selection in view as the arrow keys move it.
The selection is held as a *key*, so rows inserted or removed above it leave
it on the same row rather than the same position.

```c
#define CTL2_TABLE_COLUMNS 8     /* columns at most */
#define CTL2_TABLE_CELL    64    /* a heading or cell, terminator included */

#define CTL2ALIGN_LEFT   0
#define CTL2ALIGN_RIGHT  1
#define CTL2ALIGN_CENTER 2

#define CTL2_TABLE_NOROW ((WORD32)-1)
```

### CmnCtl2_CreateTable

```c
HANDLE CmnCtl2_CreateTable(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                           WORD32 VisibleRows, WORD32 IconSize);
```

cmnctl2.dll · export `CmnCtl2_CreateTable1` · SDK 10

A table `W` wide, `VisibleRows` rows tall below its heading strip. `IconSize`
is the edge of the square picture each row carries, or zero for none; it is
fixed, since the row height comes from it.

### CmnCtl2_AddColumn

```c
BOOL CmnCtl2_AddColumn(HANDLE Table, PSTR Heading, WORD32 Width, WORD32 Align);
```

cmnctl2.dll · export `CmnCtl2_AddColumn1` · SDK 10

Declares the next column. The heading is copied. **Returns** `FALSE` past
`CTL2_TABLE_COLUMNS`.

### CmnCtl2_SetColumnWidth

```c
BOOL CmnCtl2_SetColumnWidth(HANDLE Table, WORD32 Column, WORD32 Width);
```

cmnctl2.dll · export `CmnCtl2_SetColumnWidth1` · SDK 10

Changes a column's width, by the index it was added at, for a table widened
by a resize.

### CmnCtl2_SetRowCount

```c
BOOL CmnCtl2_SetRowCount(HANDLE Table, WORD32 Rows);
```

cmnctl2.dll · export `CmnCtl2_SetRowCount1` · SDK 10

How many rows there are. Growing keeps the existing rows and gives the new
ones their index as a key; shrinking keeps the storage.

### CmnCtl2_GetRowCount

```c
WORD32 CmnCtl2_GetRowCount(HANDLE Table);
```

cmnctl2.dll · export `CmnCtl2_GetRowCount1` · SDK 10

### CmnCtl2_SetCell

```c
VOID CmnCtl2_SetCell(HANDLE Table, WORD32 Row, WORD32 Column, PSTR Text);
```

cmnctl2.dll · export `CmnCtl2_SetCell1` · SDK 10

One cell's text, copied, cut short at `CTL2_TABLE_CELL - 1` characters.

### CmnCtl2_GetCell

```c
PSTR CmnCtl2_GetCell(HANDLE Table, WORD32 Row, WORD32 Column);
```

cmnctl2.dll · export `CmnCtl2_GetCell1` · SDK 10

One cell's text, as the table's own storage.

### CmnCtl2_SetRowKey

```c
VOID CmnCtl2_SetRowKey(HANDLE Table, WORD32 Row, WORD64 Key);
```

cmnctl2.dll · export `CmnCtl2_SetRowKey1` · SDK 10

What a row *is*, as opposed to where it is — an identifier the application
chooses. Rows given no key keep their index.

### CmnCtl2_GetRowKey

```c
WORD64 CmnCtl2_GetRowKey(HANDLE Table, WORD32 Row);
```

cmnctl2.dll · export `CmnCtl2_GetRowKey1` · SDK 10

### CmnCtl2_SetRowIcon

```c
VOID CmnCtl2_SetRowIcon(HANDLE Table, WORD32 Row, PVOID Icon);
```

cmnctl2.dll · export `CmnCtl2_SetRowIcon1` · SDK 10

The picture at the start of a row, at the table's icon size. Borrowed, not
copied, so it must outlive the table. `NULL` removes it.

### CmnCtl2_GetSelectedRow

```c
WORD32 CmnCtl2_GetSelectedRow(HANDLE Table);
```

cmnctl2.dll · export `CmnCtl2_GetSelectedRow1` · SDK 10

The row holding the selected key, or `CTL2_TABLE_NOROW` when nothing is
selected or the selected row has gone.

### CmnCtl2_GetSelectedKey

```c
WORD64 CmnCtl2_GetSelectedKey(HANDLE Table);
```

cmnctl2.dll · export `CmnCtl2_GetSelectedKey1` · SDK 10

The selected key.

### CmnCtl2_SelectRow

```c
VOID CmnCtl2_SelectRow(HANDLE Table, WORD32 Row);
```

cmnctl2.dll · export `CmnCtl2_SelectRow1` · SDK 10

Selects a row, by position. A row past the end clears the selection.

---

## Trees

An indented list of nodes, given in document order with a depth each. The tree
works out what is on screen — a node shows only when every shallower node
above it is expanded — and owns the expanding, so opening a folder needs
nothing from the application.

```c
CmnCtl2_SetNodeCount(Tree, Total);
CmnCtl2_SetNode(Tree, i, Depth, Name, Icon, IsFolder);
```

Clicking the marker at the start of an expandable node opens or closes it;
clicking anywhere else on it selects it. Selection is held by key, as a
table's is.

```c
#define CTL2_TREE_NONODE ((WORD32)-1)
```

### CmnCtl2_CreateTree

```c
HANDLE CmnCtl2_CreateTree(HANDLE Window, WORD32 X, WORD32 Y, WORD32 W,
                          WORD32 VisibleRows, WORD32 IconSize);
```

cmnctl2.dll · export `CmnCtl2_CreateTree1` · SDK 10

A tree `W` wide and `VisibleRows` rows tall, with icons of `IconSize`, or
none for zero.

### CmnCtl2_SetNodeCount

```c
BOOL CmnCtl2_SetNodeCount(HANDLE Tree, WORD32 Nodes);
```

cmnctl2.dll · export `CmnCtl2_SetNodeCount1` · SDK 10

How many nodes there are. Growing keeps the existing nodes, which are open,
and gives new ones their index as a key.

### CmnCtl2_GetNodeCount

```c
WORD32 CmnCtl2_GetNodeCount(HANDLE Tree);
```

cmnctl2.dll · export `CmnCtl2_GetNodeCount1` · SDK 10

### CmnCtl2_SetNode

```c
VOID CmnCtl2_SetNode(HANDLE Tree, WORD32 Node, WORD32 Depth, PSTR Text,
                     PVOID Icon, BOOL Expandable);
```

cmnctl2.dll · export `CmnCtl2_SetNode1` · SDK 10

One node: its depth (0 at the left), its text (copied), its icon (borrowed),
and whether it can be opened, whether or not anything follows it at a greater
depth.

### CmnCtl2_GetNodeText

```c
PSTR CmnCtl2_GetNodeText(HANDLE Tree, WORD32 Node);
```

cmnctl2.dll · export `CmnCtl2_GetNodeText1` · SDK 10

### CmnCtl2_SetNodeKey

```c
VOID CmnCtl2_SetNodeKey(HANDLE Tree, WORD32 Node, WORD64 Key);
```

cmnctl2.dll · export `CmnCtl2_SetNodeKey1` · SDK 10

### CmnCtl2_GetNodeKey

```c
WORD64 CmnCtl2_GetNodeKey(HANDLE Tree, WORD32 Node);
```

cmnctl2.dll · export `CmnCtl2_GetNodeKey1` · SDK 10

### CmnCtl2_SetNodeExpanded

```c
VOID CmnCtl2_SetNodeExpanded(HANDLE Tree, WORD32 Node, BOOL Expanded);
```

cmnctl2.dll · export `CmnCtl2_SetNodeExpanded1` · SDK 10

Opens or closes a node from code.

### CmnCtl2_IsNodeExpanded

```c
BOOL CmnCtl2_IsNodeExpanded(HANDLE Tree, WORD32 Node);
```

cmnctl2.dll · export `CmnCtl2_IsNodeExpanded1` · SDK 10

### CmnCtl2_GetSelectedNode

```c
WORD32 CmnCtl2_GetSelectedNode(HANDLE Tree);
```

cmnctl2.dll · export `CmnCtl2_GetSelectedNode1` · SDK 10

The selected node's position, or `CTL2_TREE_NONODE`.

### CmnCtl2_GetSelectedNodeKey

```c
WORD64 CmnCtl2_GetSelectedNodeKey(HANDLE Tree);
```

cmnctl2.dll · export `CmnCtl2_GetSelectedNodeKey1` · SDK 10

### CmnCtl2_SelectNode

```c
VOID CmnCtl2_SelectNode(HANDLE Tree, WORD32 Node);
```

cmnctl2.dll · export `CmnCtl2_SelectNode1` · SDK 10

Selects a node by position.

### CmnCtl2_GetVisibleNodes

```c
WORD32 CmnCtl2_GetVisibleNodes(HANDLE Tree);
```

cmnctl2.dll · export `CmnCtl2_GetVisibleNodes1` · SDK 10

How many nodes are on screen, which is fewer than the count when anything is
closed.
