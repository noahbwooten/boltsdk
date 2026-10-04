#pragma once
/*
cmnctl2.h
BoltOS Usermode Include Declarations
The retained-mode common controls

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"
#include "wndproc2.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define CmnCtl2_CreateWindow       CmnCtl2_CreateWindow1
#define CmnCtl2_DestroyWindow      CmnCtl2_DestroyWindow1
#define CmnCtl2_DestroyControls    CmnCtl2_DestroyControls1
#define CmnCtl2_CreatePanel        CmnCtl2_CreatePanel1
#define CmnCtl2_CreateLabel        CmnCtl2_CreateLabel1
#define CmnCtl2_CreateButton       CmnCtl2_CreateButton1
#define CmnCtl2_CreateCheckbox     CmnCtl2_CreateCheckbox1
#define CmnCtl2_CreateTextInput    CmnCtl2_CreateTextInput1
#define CmnCtl2_CreateTextArea     CmnCtl2_CreateTextArea1
#define CmnCtl2_CreateIntInput     CmnCtl2_CreateIntInput1
#define CmnCtl2_CreateComboBox     CmnCtl2_CreateComboBox1
#define CmnCtl2_CreateTabBar       CmnCtl2_CreateTabBar1
#define CmnCtl2_CreateMeter        CmnCtl2_CreateMeter1
#define CmnCtl2_CreateGraph        CmnCtl2_CreateGraph1
#define CmnCtl2_CreateScrollBar    CmnCtl2_CreateScrollBar1
#define CmnCtl2_SetScrollRange     CmnCtl2_SetScrollRange1
#define CmnCtl2_GetScrollPos       CmnCtl2_GetScrollPos1
#define CmnCtl2_SetScrollPos       CmnCtl2_SetScrollPos1
#define CmnCtl2_DestroyControl     CmnCtl2_DestroyControl1
#define CmnCtl2_CreateTable        CmnCtl2_CreateTable1
#define CmnCtl2_AddColumn          CmnCtl2_AddColumn1
#define CmnCtl2_SetColumnWidth     CmnCtl2_SetColumnWidth1
#define CmnCtl2_SetRowCount        CmnCtl2_SetRowCount1
#define CmnCtl2_GetRowCount        CmnCtl2_GetRowCount1
#define CmnCtl2_SetCell            CmnCtl2_SetCell1
#define CmnCtl2_GetCell            CmnCtl2_GetCell1
#define CmnCtl2_SetRowKey          CmnCtl2_SetRowKey1
#define CmnCtl2_GetRowKey          CmnCtl2_GetRowKey1
#define CmnCtl2_SetRowIcon         CmnCtl2_SetRowIcon1
#define CmnCtl2_GetSelectedRow     CmnCtl2_GetSelectedRow1
#define CmnCtl2_GetSelectedKey     CmnCtl2_GetSelectedKey1
#define CmnCtl2_SelectRow          CmnCtl2_SelectRow1
#define CmnCtl2_GetHeight          CmnCtl2_GetHeight1
#define CmnCtl2_CreateTree         CmnCtl2_CreateTree1
#define CmnCtl2_SetNodeCount       CmnCtl2_SetNodeCount1
#define CmnCtl2_GetNodeCount       CmnCtl2_GetNodeCount1
#define CmnCtl2_SetNode            CmnCtl2_SetNode1
#define CmnCtl2_GetNodeText        CmnCtl2_GetNodeText1
#define CmnCtl2_SetNodeKey         CmnCtl2_SetNodeKey1
#define CmnCtl2_GetNodeKey         CmnCtl2_GetNodeKey1
#define CmnCtl2_SetNodeExpanded    CmnCtl2_SetNodeExpanded1
#define CmnCtl2_IsNodeExpanded     CmnCtl2_IsNodeExpanded1
#define CmnCtl2_GetSelectedNode    CmnCtl2_GetSelectedNode1
#define CmnCtl2_GetSelectedNodeKey CmnCtl2_GetSelectedNodeKey1
#define CmnCtl2_SelectNode         CmnCtl2_SelectNode1
#define CmnCtl2_GetVisibleNodes    CmnCtl2_GetVisibleNodes1
#define CmnCtl2_SetText            CmnCtl2_SetText1
#define CmnCtl2_GetText            CmnCtl2_GetText1
#define CmnCtl2_SetPlaceholder     CmnCtl2_SetPlaceholder1
#define CmnCtl2_SetValue           CmnCtl2_SetValue1
#define CmnCtl2_GetValue           CmnCtl2_GetValue1
#define CmnCtl2_SetEnabled         CmnCtl2_SetEnabled1
#define CmnCtl2_SetBackground      CmnCtl2_SetBackground1
#define CmnCtl2_SetFocus           CmnCtl2_SetFocus1
#define CmnCtl2_SetHidden          CmnCtl2_SetHidden1
#define CmnCtl2_SetPosition        CmnCtl2_SetPosition1
#define CmnCtl2_SetSize            CmnCtl2_SetSize1
#define CmnCtl2_PushSample         CmnCtl2_PushSample1
#define CmnCtl2_WasClicked         CmnCtl2_WasClicked1
#define CmnCtl2_WasDoubleClicked   CmnCtl2_WasDoubleClicked1
#define CmnCtl2_GetSelection       CmnCtl2_GetSelection1
#define CmnCtl2_Paint              CmnCtl2_Paint1
#define CmnCtl2_Dispatch           CmnCtl2_Dispatch1
#define CmnCtl2_GetColors          CmnCtl2_GetColors1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

/*
 cmnctl2: controls that exist, rather than controls that are drawn. A control
 is created once and kept, owns its text, position and state, and reports when
 it changes, which lets the window manager skip painting a window with nothing
 new to show. cmnctl is frozen and unaffected; the two share no code.

     HANDLE Window = CmnCtl2_CreateWindow(...);
     HANDLE Save   = CmnCtl2_CreateButton(Window, 10, 40, 80, 30, "Save");
     ...
     case MSG_PAINT:
         CmnCtl2_Paint(Window);
         return 0;
     default:
         if (CmnCtl2_Dispatch(Window, Message, Param1, Param2))
             return 0;
         if (CmnCtl2_WasClicked(Save))
             ...

 Coordinates are window-relative: (0, 0) is the corner of the window and stays
 there when it is dragged.
 */

/* Exported when building the library, imported by everything else. _CMNCTL2 is
   defined only for this module's own sources; without the split the library
   would declare its own functions as imported and then define them. */
#ifdef _CMNCTL2
#define CMNCTL2_EXPORT __declspec(dllexport)
#else
#define CMNCTL2_EXPORT __declspec(dllimport)
#endif

/* -- Windows -- */

/*
 Create a window, with its backing surface. A thin wrapper over the window
 manager rather than a second way of making a window, so an application written
 against cmnctl2 has one library to talk to instead of two.
 */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateWindow(WORD32 X, WORD32 Y, WORD32 W, WORD32 H,
	PSTR Title, PVOID Icon);

/* Destroy a window and every control on it. Controls are owned by their window;
   an application does not have to take them down one at a time, and one that
   tries to keep a control past its window is holding a stale handle. */
CMNCTL2_EXPORT VOID CmnCtl2_DestroyWindow(HANDLE Window);

/*
 Take down the controls but leave the window. What an application calls from
 MSG_QUIT, where the window manager closes the window itself once the message
 returns: without this the controls outlive it, holding their handles and their
 text for as long as the machine runs.
 */
CMNCTL2_EXPORT VOID CmnCtl2_DestroyControls(HANDLE Window);

/* -- Controls -- */

/* What a control is. Stored in the handle; an application never needs it
   except to read a control's kind back. */
#define CTL2_LABEL     1
#define CTL2_BUTTON    2
#define CTL2_CHECKBOX  3
#define CTL2_TEXTINPUT 4
#define CTL2_INTINPUT  5
#define CTL2_COMBOBOX  6
#define CTL2_TABBAR    7
#define CTL2_METER     8
#define CTL2_GRAPH     9
#define CTL2_TEXTAREA  10
#define CTL2_TABLE     11
#define CTL2_TREE      12
#define CTL2_PANEL     13
#define CTL2_SCROLLBAR 14

/* Flags. */
#define CTL2F_DISABLED 0x0001  /* drawn dimmed, ignores input */
#define CTL2F_HIDDEN   0x0002  /* not drawn, ignores input */
#define CTL2F_NOFOCUS  0x0004  /* skipped by tab; labels and meters set this */

/* Set on a control whose panel is hidden. Kept apart from CTL2F_HIDDEN so that
   showing a page does not reveal the controls the page itself had hidden. */
#define CTL2F_PANELHIDDEN 0x0008

/* Drawn without the control's own background: the calling application paints
   whatever it wants underneath and the control puts only its text and its
   caret over it. For a control sitting on a surface that is not a control
   colour -- a sticky note's paper, say. The ink goes down dark, since a
   caller painting its own background is usually painting paper. */
#define CTL2F_NOBACKGROUND 0x0010

/* -- Panels -- */

/*
 A group of controls that can be shown, hidden and destroyed together, and
 which places what is put on it relative to its own corner. Every create call
 above takes a panel wherever it takes a window:

     HANDLE Page = CmnCtl2_CreatePanel(Window, 15, 82, 600, 400);
     CmnCtl2_CreateButton(Page, 5, 10, 80, 24, "Apply");   -- at 20, 92
     CmnCtl2_SetHidden(Page, TRUE);                        -- the button too

 A panel is a group rather than a container: its members sit in the window's
 one list, so painting order, hit testing and the tab order are exactly what
 they would be without it. It draws nothing itself and cannot be clicked, and
 nothing is clipped to it. Its position is fixed at creation.

 Destroying a panel destroys its members. Destroying a window destroys both.
 */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreatePanel(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 H);

CMNCTL2_EXPORT HANDLE CmnCtl2_CreateLabel(HANDLE Window, WORD32 X, WORD32 Y, PSTR Text);
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateButton(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 H, PSTR Text);
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateCheckbox(HANDLE Window, WORD32 X, WORD32 Y,
	PSTR Text, BOOL Checked);

/* Capacity is how much text the control will hold, including the terminator.
   Owned by the control and freed with it. */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateTextInput(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 Capacity);

/* A multi-line text area. Lines is how many rows are visible; the buffer is
   Capacity bytes and wraps at the control's width. */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateTextArea(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 Lines, WORD32 Capacity);

CMNCTL2_EXPORT HANDLE CmnCtl2_CreateIntInput(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, INT Min, INT Max, INT Value);

/* Items must stay alive for as long as the control does, since they are not
   copied. String literals, or a table the application keeps. */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateComboBox(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, PSTR Title, WORD32 Count, PSTR* Items);
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateTabBar(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 Count, PSTR* Names);

CMNCTL2_EXPORT HANDLE CmnCtl2_CreateMeter(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 H);
/* Max is the full-scale reading, or zero to scale to the largest sample the
   graph is holding. Zero is what a trace in kilobytes wants, where no fixed
   full scale stays right for long. */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateGraph(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 H, WORD32 Samples, WORD32 Max);

/* -- Scrollbars -- */

/*
 A vertical scrollbar, with a button at each end and a thumb between them. The
 width is fixed and the same as the one a table, a tree and a text area grow for
 themselves, so a bar placed by hand beside one of those lines up with it.

     HANDLE Bar = CmnCtl2_CreateScrollBar(Window, 400, 60, 240);
     CmnCtl2_SetScrollRange(Bar, 500, 12);   -- 500 items, 12 of them showing

 Position is the first item shown, and stops with the last item at the bottom
 rather than at the top. A control that scrolls itself needs none of this: it
 grows its own bar and answers the wheel without being asked.
 */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateScrollBar(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 H);

/* Total items and how many are on screen. A bar showing everything is drawn
   flat, with no thumb, and refuses to move. */
CMNCTL2_EXPORT VOID CmnCtl2_SetScrollRange(HANDLE Bar, WORD32 Total,
	WORD32 Visible);

CMNCTL2_EXPORT WORD32 CmnCtl2_GetScrollPos(HANDLE Bar);
CMNCTL2_EXPORT VOID CmnCtl2_SetScrollPos(HANDLE Bar, WORD32 Position);

/* The width every bar in the system takes, so an application laying out beside
   one does not have to guess. */
#define CTL2_SCROLLBAR_W 14

/* Takes a panel's members with it when given a panel. */
CMNCTL2_EXPORT VOID CmnCtl2_DestroyControl(HANDLE Control);

/* -- Tables -- */

/*
 A fixed table: columns declared once, rows written into storage the table
 owns. Cell text is copied and compared before it is written, so a refresh that
 changes two figures out of two hundred repaints because of those two.

     HANDLE T = CmnCtl2_CreateTable(Window, 10, 76, 600, 12, 0);
     CmnCtl2_AddColumn(T, "Name", 220, CTL2ALIGN_LEFT);
     CmnCtl2_AddColumn(T, "Memory", 90, CTL2ALIGN_RIGHT);
     ...
     CmnCtl2_SetRowCount(T, Count);
     CmnCtl2_SetRowKey(T, i, Pid);
     CmnCtl2_SetCell(T, i, 0, Name);

 Rows scroll. A table taller than its room grows a bar down its right edge,
 answers the wheel and the page keys when it has the focus, and keeps the
 selection in view as the arrow keys move it.
 */

/* The most columns a table holds, and the most a heading or a cell stores,
   including the terminator. Both are fixed, so a row is one stride and the
   whole grid is one allocation. */
#define CTL2_TABLE_COLUMNS 8
#define CTL2_TABLE_CELL    64

/* Where a cell's text sits in its column. */
#define CTL2ALIGN_LEFT   0
#define CTL2ALIGN_RIGHT  1
#define CTL2ALIGN_CENTER 2

/* No row: nothing is selected, or the pointer is not over one. */
#define CTL2_TABLE_NOROW ((WORD32)-1)

/*
 VisibleRows sets the height, in rows below the heading strip. IconSize is the
 edge of the square image each row carries, or zero for a table with no icons.
 It is fixed at creation because the row height and the first column's indent
 both come from it, and a table whose rows moved when one gained an icon would
 be a table that relaid itself out under the reader.
 */
CMNCTL2_EXPORT HANDLE CmnCtl2_CreateTable(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 VisibleRows, WORD32 IconSize);

/* Declare a column. The heading is copied. Returns false past
   CTL2_TABLE_COLUMNS. A column added once rows exist starts empty. */
CMNCTL2_EXPORT BOOL CmnCtl2_AddColumn(HANDLE Table, PSTR Heading, WORD32 Width,
	WORD32 Align);

/* Change one column's width, by the index it was added at. What a table widened
   by a resize needs, since the table never moves its own columns. */
CMNCTL2_EXPORT BOOL CmnCtl2_SetColumnWidth(HANDLE Table, WORD32 Column,
	WORD32 Width);

/*
 How many rows there are. Growing keeps what the existing rows hold and gives
 the new ones their index as a key; shrinking keeps the storage, so a count
 that moves up and down every second does not reallocate every second.
 */
CMNCTL2_EXPORT BOOL CmnCtl2_SetRowCount(HANDLE Table, WORD32 Rows);
CMNCTL2_EXPORT WORD32 CmnCtl2_GetRowCount(HANDLE Table);

/* Cell text, copied into the table. Writing what is already there marks
   nothing, which is what makes a once-a-second refresh cheap. */
CMNCTL2_EXPORT VOID CmnCtl2_SetCell(HANDLE Table, WORD32 Row, WORD32 Column,
	PSTR Text);
CMNCTL2_EXPORT PSTR CmnCtl2_GetCell(HANDLE Table, WORD32 Row, WORD32 Column);

/*
 What a row is, as opposed to where it is. Selection is held as a key, so a
 refresh that inserts or removes rows above the selected one leaves the
 selection on the same row rather than on the same position. Rows given no key
 keep their index, which makes an application that ignores keys behave as
 though selection followed the position.
 */
CMNCTL2_EXPORT VOID CmnCtl2_SetRowKey(HANDLE Table, WORD32 Row, WORD64 Key);
CMNCTL2_EXPORT WORD64 CmnCtl2_GetRowKey(HANDLE Table, WORD32 Row);

/* The image drawn at the start of the row, at the size the table was created
   with. Borrowed, not copied, so it must outlive the table. Null removes it. */
CMNCTL2_EXPORT VOID CmnCtl2_SetRowIcon(HANDLE Table, WORD32 Row, PVOID Icon);

/* CTL2_TABLE_NOROW when nothing is selected, or when the selected row is no
   longer in the table. CmnCtl2_GetSelection returns the same thing. */
CMNCTL2_EXPORT WORD32 CmnCtl2_GetSelectedRow(HANDLE Table);
CMNCTL2_EXPORT WORD64 CmnCtl2_GetSelectedKey(HANDLE Table);
CMNCTL2_EXPORT VOID CmnCtl2_SelectRow(HANDLE Table, WORD32 Row);

/* How tall the control ended up, so an application can put something directly
   beneath it without repeating the arithmetic. */
CMNCTL2_EXPORT WORD32 CmnCtl2_GetHeight(HANDLE Control);

/* -- Trees -- */

/*
 An indented list of nodes, given in document order with a depth each. The tree
 works out what is on screen: a node is drawn only when every shallower node
 above it is expanded. The application supplies the whole tree once and the
 control owns the expansion, so opening a folder needs nothing from it.

     CmnCtl2_SetNodeCount(T, Total);
     CmnCtl2_SetNode(T, i, Depth, Name, Icon, IsDirectory);

 Clicking the marker at the start of an expandable node opens or closes it.
 Clicking anywhere else on the node selects it.
 */

/* No node: nothing is selected, or the pointer is not over one. */
#define CTL2_TREE_NONODE ((WORD32)-1)

CMNCTL2_EXPORT HANDLE CmnCtl2_CreateTree(HANDLE Window, WORD32 X, WORD32 Y,
	WORD32 W, WORD32 VisibleRows, WORD32 IconSize);

/* Growing keeps the nodes that were there, including which are open, and gives
   the new ones their index as a key. */
CMNCTL2_EXPORT BOOL CmnCtl2_SetNodeCount(HANDLE Tree, WORD32 Nodes);
CMNCTL2_EXPORT WORD32 CmnCtl2_GetNodeCount(HANDLE Tree);

/* One node. Text is copied and compared before it is written; the icon is
   borrowed. Expandable marks a node that can be opened, whether or not
   anything follows it at a greater depth. */
CMNCTL2_EXPORT VOID CmnCtl2_SetNode(HANDLE Tree, WORD32 Node, WORD32 Depth,
	PSTR Text, PVOID Icon, BOOL Expandable);
CMNCTL2_EXPORT PSTR CmnCtl2_GetNodeText(HANDLE Tree, WORD32 Node);

CMNCTL2_EXPORT VOID CmnCtl2_SetNodeKey(HANDLE Tree, WORD32 Node, WORD64 Key);
CMNCTL2_EXPORT WORD64 CmnCtl2_GetNodeKey(HANDLE Tree, WORD32 Node);

CMNCTL2_EXPORT VOID CmnCtl2_SetNodeExpanded(HANDLE Tree, WORD32 Node,
	BOOL Expanded);
CMNCTL2_EXPORT BOOL CmnCtl2_IsNodeExpanded(HANDLE Tree, WORD32 Node);

/* Selection is held by key, the same way a table holds it. CTL2_TREE_NONODE
   when nothing is selected or the selected node is gone. */
CMNCTL2_EXPORT WORD32 CmnCtl2_GetSelectedNode(HANDLE Tree);
CMNCTL2_EXPORT WORD64 CmnCtl2_GetSelectedNodeKey(HANDLE Tree);
CMNCTL2_EXPORT VOID CmnCtl2_SelectNode(HANDLE Tree, WORD32 Node);

/* How many nodes are currently on screen, which is not the node count when
   anything is collapsed. */
CMNCTL2_EXPORT WORD32 CmnCtl2_GetVisibleNodes(HANDLE Tree);

/* -- State -- */

/*
 Every setter marks the window as needing repainting, which is why an
 application never has to invalidate anything. Setting a value to what it
 already was marks nothing, so an unchanged value costs nothing.
 */
CMNCTL2_EXPORT VOID CmnCtl2_SetText(HANDLE Control, PSTR Text);
CMNCTL2_EXPORT PSTR CmnCtl2_GetText(HANDLE Control);

/*
 What a text input shows, and is worth, while nothing has been typed into it.
 Drawn dimmed and returned by CmnCtl2_GetText, so it is the default value
 rather than a hint about one, and a caller never tests for empty.
 */
CMNCTL2_EXPORT VOID CmnCtl2_SetPlaceholder(HANDLE Control, PSTR Text);

/* Meaning depends on the kind: checked for a checkbox, the number for an
   integer input, the selection for a combo box or tab bar, the reading for a
   meter, the percentage for a graph. */
CMNCTL2_EXPORT VOID CmnCtl2_SetValue(HANDLE Control, WORD32 Value);
CMNCTL2_EXPORT WORD32 CmnCtl2_GetValue(HANDLE Control);

CMNCTL2_EXPORT VOID CmnCtl2_SetEnabled(HANDLE Control, BOOL Enabled);

/* Whether the control paints its own background. On by default; off, the
   application paints what is underneath and the control keeps to its text.
   See CTL2F_NOBACKGROUND. */
CMNCTL2_EXPORT VOID CmnCtl2_SetBackground(HANDLE Control, BOOL Painted);

/* Give a control the keyboard, as clicking it would. For a page that opens to
   ask for something typed, so the first keystroke lands in the box rather than
   nowhere. Ignored for a control that never takes the focus. */
CMNCTL2_EXPORT VOID CmnCtl2_SetFocus(HANDLE Control);

/* Hides a panel's members along with it, without disturbing which of them the
   page had hidden on its own account. */
CMNCTL2_EXPORT VOID CmnCtl2_SetHidden(HANDLE Control, BOOL Hidden);
/* In the coordinates the control was created in, which for a control on a
   panel means relative to the panel. */
CMNCTL2_EXPORT VOID CmnCtl2_SetPosition(HANDLE Control, WORD32 X, WORD32 Y);

/*
 Resize a control, for a window laying itself out again after MSG_RESIZE. A
 text area, table or tree measures its height in rows, so the height given is
 rounded down to a whole number of them; ask with CmnCtl2_GetHeight after.
 */
CMNCTL2_EXPORT VOID CmnCtl2_SetSize(HANDLE Control, WORD32 W, WORD32 H);

/* Push a reading into a graph. Marks the window dirty, so a graph fed once a
   second repaints once a second and not sixty times. */
CMNCTL2_EXPORT VOID CmnCtl2_PushSample(HANDLE Graph, WORD32 Value);

/*
 Was this control activated since the last time anybody asked? Reads and
 clears, so it is safe to call once a frame and has the shape of the
 immediate-mode call it replaces.
 */
CMNCTL2_EXPORT BOOL CmnCtl2_WasClicked(HANDLE Control);

/*
 Whether that click was the second of a pair, on the same control and in the
 same place. Reported alongside the ordinary click and not instead of it, so a
 double click is a click that is also a double: ask both.
 */
CMNCTL2_EXPORT BOOL CmnCtl2_WasDoubleClicked(HANDLE Control);

/* Which tab, item or row is selected. Equivalent to CmnCtl2_GetValue, except
   on a table, where the row is derived from the selected key. */
CMNCTL2_EXPORT WORD32 CmnCtl2_GetSelection(HANDLE Control);

/* -- The framework -- */

/* Draw every control on the window. Call from MSG_PAINT and nowhere else; see
   the note on MSG_PAINT in wndproc2.h. */
CMNCTL2_EXPORT VOID CmnCtl2_Paint(HANDLE Window);

/*
 Give a message to the controls first. Returns non-zero when a control consumed
 it, so an application can skip its own handling: a keystroke that went into a
 text box should not also be a shortcut.
 */
CMNCTL2_EXPORT WORD64 CmnCtl2_Dispatch(HANDLE Window, WORD32 Message,
	WORD64 Param1, WORD64 Param2);

/* The colours, the same table cmnctl and the window manager use. */
CMNCTL2_EXPORT PVOID CmnCtl2_GetColors(VOID);

#endif /* BOLTSDK_APIV_10 */
