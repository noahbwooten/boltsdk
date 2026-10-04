#pragma once
/*
 calc.h
 (c)Noah Wooten 2023 - 2026, All Rights Reserved

 The calculator's pieces, shared between main.c, which is what the system
 calls, and calc.c, which is the calculator.
*/

#include <user.h>

void CalcInit(void);
void CalcHalt(void);

/* The window's procedure. Exported, since the window manager finds it in the
   module's export table by name; see wndproc2.h. */
__declspec(dllexport) WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message,
	WORD64 Param1, WORD64 Param2);
