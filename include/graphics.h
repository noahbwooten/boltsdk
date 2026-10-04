#pragma once
/*
graphics.h
BoltOS Usermode Include Declarations
User-Mode Graphics Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

/*
 The Bolt API names this header declares, for an application built against
 the SDK: the name an application writes, and the export it calls. See
 boltsdk.h. The system's own modules leave BOLTSDK_APIV undefined and call
 the plain names.
 */
#ifdef BOLTSDK_APIV
#define Ge_Color             Ge_Color1
#define Ge_SetClip           Ge_SetClip1
#define Ge_ClearClip         Ge_ClearClip1
#define Ge_GetClip           Ge_GetClip1
#define Ge_DrawFilled        Ge_DrawFilled1
#define Ge_DrawOutline       Ge_DrawOutline1
#define Ge_DrawBuffer        Ge_DrawBuffer1
#define Ge_DrawText          Ge_DrawText1
#define Ge_GetTextLength     Ge_GetTextLength1
#define Ge_DrawCircleOutline Ge_DrawCircleOutline1
#define Ge_DrawCircleFilled  Ge_DrawCircleFilled1
#define Ge_StretchBitmap     Ge_StretchBitmap1
#define Ge_GetsBitForBmp     Ge_GetsBitForBmp1
#endif

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

// declarations
#define GEFONT_NORMAL 0
#define GEFONT_BOLD 1
#define GEFONT_OUTLINE 2

// color functions
UMI_FUNCTION WORD32 Ge_Color(BYTE r, BYTE g, BYTE b, BYTE a);

/*
 Hard clipping. Every primitive is confined to the rectangle in force, which is
 what lets a control draw rows past its own edge and have them cut off rather
 than counted out beforehand. A zero width or height means no clip.
 */
UMI_FUNCTION VOID Ge_SetClip(WORD32 X, WORD32 Y, WORD32 W, WORD32 H);
UMI_FUNCTION VOID Ge_ClearClip(VOID);
UMI_FUNCTION VOID Ge_GetClip(PWORD32 X, PWORD32 Y, PWORD32 W, PWORD32 H);

// rendering functions
UMI_FUNCTION VOID Ge_DrawFilled(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Color);
UMI_FUNCTION VOID Ge_DrawOutline(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, WORD32 Color);
UMI_FUNCTION VOID Ge_DrawBuffer(WORD32 X, WORD32 Y, WORD32 W, WORD32 H, PVOID Data);
UMI_FUNCTION VOID Ge_DrawText(WORD32 X, WORD32 Y, PSTR Text, WORD32 Color, WORD8 Font);
UMI_FUNCTION VOID Ge_GetTextLength(PSTR Text, PWORD32 OutLength, WORD32 Font);
UMI_FUNCTION VOID Ge_DrawCircleOutline(WORD32 X, WORD32 Y, WORD32 R, WORD32 Color);
UMI_FUNCTION VOID Ge_DrawCircleFilled(WORD32 X, WORD32 Y, WORD32 R, WORD32 Color);

// image manipulation function
UMI_FUNCTION VOID Ge_StretchBitmap(PVOID Bits, WORD32 OldWidth, WORD32 OldHeight,
    WORD32 NewWidth, WORD32 NewHeight, PVOID OutBits);
UMI_FUNCTION VOID Ge_GetsBitForBmp(PVOID Bmp, PVOID* Bits, WORD32* BitSize);

#endif /* BOLTSDK_APIV_10 */
