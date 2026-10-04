#pragma once
#include "umbase.h"
#include "wndmgr.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define CmnCtl_GetColors              CmnCtl_GetColors1
#define CmnCtl_DrawButton             CmnCtl_DrawButton1
#define CmnCtl_DrawCheckbox           CmnCtl_DrawCheckbox1
#define CmnCtl_DrawComboBox           CmnCtl_DrawComboBox1
#define CmnCtl_DrawTextInput          CmnCtl_DrawTextInput1
#define CmnCtl_DrawTextInputML        CmnCtl_DrawTextInputML1
#define CmnCtl_DrawIntInput           CmnCtl_DrawIntInput1
#define CmnCtl_ClockTextInputML       CmnCtl_ClockTextInputML1
#define CmnCtl_CreateTabController    CmnCtl_CreateTabController1
#define CmnCtl_DestroyTabController   CmnCtl_DestroyTabController1
#define CmnCtl_DrawTabController      CmnCtl_DrawTabController1
#define CmnCtl_EndTabHandler          CmnCtl_EndTabHandler1
#define CmnCtl_DrawTab                CmnCtl_DrawTab1
#define CmnCtl_DrawDropMenu           CmnCtl_DrawDropMenu1
#define CmnCtl_DrawDropItem           CmnCtl_DrawDropItem1
#define CmnCtl_DrawDropItemSelectable CmnCtl_DrawDropItemSelectable1
#define CmnCtl_DrawMeter              CmnCtl_DrawMeter1
#define CmnCtl_CreateGraph            CmnCtl_CreateGraph1
#define CmnCtl_DestroyGraph           CmnCtl_DestroyGraph1
#define CmnCtl_PushGraphSample        CmnCtl_PushGraphSample1
#define CmnCtl_DrawGraph              CmnCtl_DrawGraph1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define UMI_EXPORT __declspec(dllexport)

typedef struct _TAB_CTL {
	WORD32 TabCount;
	WORD32 SelectedTab;
	WORD32* TabSizes;
	PSTR* TabNames;
}TAB_CTL, * PTAB_CTL;

/*
 A history graph's state. The control keeps the samples and the owner keeps the
 clock, so the period covered is SampleCount multiplied by how often the owner
 pushes. Push from a real timer rather than from a redraw.
 */
#define GRAPH_GRID_PITCH 12

typedef struct _GRAPH_CTL {
	WORD32 SampleCount;      /* how many readings the ring holds */
	WORD32 Head;             /* where the next one goes */
	WORD32 Filled;           /* how many are real, until the ring wraps */
	WORD32 Max;              /* the value that reaches the top; 0 = self-scaling */
	WORD32 GridOffset;       /* travels with the samples */
	PWORD32 Samples;
}GRAPH_CTL, * PGRAPH_CTL;

typedef struct _INPUT_ML {
	char** RichText, * Text;
	int Lines;
	int MAX_TEXT_LENGTH;
	int MAX_LINE_LENGTH;
}INPUT_ML, * PINPUT_ML;

UMI_EXPORT PWNDMGR_COLORS CmnCtl_GetColors(void);

UMI_EXPORT BOOL CmnCtl_DrawButton(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, char* Text);
UMI_EXPORT BOOL CmnCtl_DrawCheckbox(WORD32 X, WORD32 Y, PSTR Title, PBOOL State);
UMI_EXPORT BOOL CmnCtl_DrawComboBox(WORD32 X, WORD32 Y, WORD32 W, PSTR Title, WORD32 ElementCnt, PSTR* Elements, PWORD32 Selected, PBOOL IsOpen);
UMI_EXPORT VOID CmnCtl_DrawTextInput(WORD32 X, WORD32 Y, WORD32 W, PSTR Text, PBOOL IsOpen, PWORD32 CursorPos);
UMI_EXPORT VOID CmnCtl_DrawTextInputML(WORD32 X, WORD32 Y, WORD32 W, WORD32 Lines, PSTR Text, PWORD32 CursorX, PWORD32 CursorY, PBOOL IsInFocus, PSTR** RichCtx);
UMI_EXPORT BOOL CmnCtl_DrawIntInput(WORD32 X, WORD32 Y, WORD32 W, INT Min, INT Max, PINT Value);
UMI_EXPORT VOID CmnCtl_ClockTextInputML(PINPUT_ML Input);
UMI_EXPORT PTAB_CTL CmnCtl_CreateTabController(WORD32 TabCount, PSTR* TabNames);
UMI_EXPORT VOID CmnCtl_DestroyTabController(PTAB_CTL TabCtl);
UMI_EXPORT VOID CmnCtl_DrawTabController(PTAB_CTL TabCtl, WORD32 X, WORD32 Y, WORD32 W);
UMI_EXPORT VOID CmnCtl_EndTabHandler(PTAB_CTL TabCtl);
UMI_EXPORT BOOL CmnCtl_DrawTab(PTAB_CTL TabCtl, WORD32 TabId);
UMI_EXPORT VOID CmnCtl_DrawDropMenu(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon, PBOOL State, PVOID StateChangedCb);
UMI_EXPORT VOID CmnCtl_DrawDropItem(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon);
UMI_EXPORT BOOL CmnCtl_DrawDropItemSelectable(WORD32 X, WORD32 Y, WORD32 W, PSTR Name, PVOID Icon, BOOL Selected);

/* A value now: a segmented column with the reading along its bottom edge.
   Suffix is the unit printed after the number ("%", "MB"); pass NULL for a
   bare column with no readout and no space reserved for one. */
UMI_EXPORT VOID CmnCtl_DrawMeter(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Value, WORD32 Max, PSTR Suffix);

/* The same value over time. Create once, push on a timer, draw every frame. */
/* Max is the full-scale value, or 0 to scale to the largest reading held, for
   quantities that sit at a small fraction of their own ceiling. */
UMI_EXPORT PGRAPH_CTL CmnCtl_CreateGraph(WORD32 SampleCount, WORD32 Max);
UMI_EXPORT VOID CmnCtl_DestroyGraph(PGRAPH_CTL Graph);
UMI_EXPORT VOID CmnCtl_PushGraphSample(PGRAPH_CTL Graph, WORD32 Value);
UMI_EXPORT VOID CmnCtl_DrawGraph(PGRAPH_CTL Graph, WORD32 X, WORD32 Y, WORD32 W, WORD32 H);

#endif /* BOLTSDK_APIV_10 */
