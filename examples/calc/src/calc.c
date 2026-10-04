/*
 calc.c
 (c)Noah Wooten 2023 - 2026, All Rights Reserved

 The calculator, on the retained controls. Sixteen buttons and a display, made
 once at MSG_INIT and left alone; the window repaints when a key is pressed and
 at no other time.
*/

#include "calc.h"
#include <stdio.h>
#include <string.h>
#include <cmnctl2.h>
#include <wndmgr.h>

/* The keypad, in reading order. The label and the operation are the same
   character, so one table serves the layout and the arithmetic. */
static const char KeyPad[16] = {
	'7', '8', '9', '/',
	'4', '5', '6', '*',
	'1', '2', '3', '-',
	'C', '0', '=', '+'
};

#define CALC_KEYW 37
#define CALC_KEYH 47
#define CALC_GAP  5

/* What the display holds, not counting the terminator. Entry stops here. */
#define CALC_DIGITS 24

/* Backspace, which has no button: it takes back the last thing typed. */
#define CALC_BACK '\b'

typedef struct _CALC_CTX {
	char DisplayStr[CALC_DIGITS + 8];

	/* The display holds an answer rather than an entry: the next digit
	   starts a new number instead of being put on the end of it. */
	BOOL Answered;

	HANDLE Display;
	HANDLE Keys[16];
	BOOL Built;
}CALC_CTX, *PCALC_CTX;

/* One per running copy. Each start of an application is its own image with
   its own variables, so two calculators do not share this. */
static PCALC_CTX CalcCtx;

void CalcInit(void) {
	CalcCtx = User_Allocate(sizeof(CALC_CTX));
	if (!CalcCtx)
		return;

	memset(CalcCtx, 0, sizeof(CALC_CTX));
}

void CalcHalt(void) {
	User_Free(CalcCtx);
	CalcCtx = NULL;
}

/* -- The arithmetic -- */

#define CALC_NUMBER_NONE     0
#define CALC_NUMBER_OK       1
#define CALC_NUMBER_OVERFLOW 2

/*
 One number out of the display. The sign is read only where a number can
 begin, since a minus in the middle is the operator that follows the number
 before it. A number longer than sixty four bits hold is reported as such
 rather than wrapped into a different one.
 */
static int CalcNumber(const char** At, long long* Out) {
	BOOL Negative = FALSE;

	if (**At == '-') {
		Negative = TRUE;
		(*At)++;
	}

	if (**At < '0' || **At > '9')
		return CALC_NUMBER_NONE;

	unsigned long long Value = 0;
	BOOL Over = FALSE;

	while (**At >= '0' && **At <= '9') {
		if (__builtin_mul_overflow(Value, 10ULL, &Value)
			|| __builtin_add_overflow(Value,
				(unsigned long long)(**At - '0'), &Value))
			Over = TRUE;

		(*At)++;
	}

	/* A long long reaches one further below zero than above it. */
	if (Over || Value > (Negative ? 9223372036854775808ULL
		: 9223372036854775807ULL))
		return CALC_NUMBER_OVERFLOW;

	*Out = Negative ? (long long)(0ULL - Value) : (long long)Value;
	return CALC_NUMBER_OK;
}

/*
 The display, worked out and written back over itself. Left to right, one
 operator and one number at a time, which is the order the buttons build it
 in: 1+2+3 is (1+2)+3. Each step is checked against what sixty four bits hold,
 and an answer that does not fit says OVERFLOW rather than something that only
 looks true.
 */
static void CalcEvaluate(char* Str) {
	const char* At = Str;
	long long Number = 0;

	/* Not a number at the front, or an empty display: nothing to answer. */
	int Read = CalcNumber(&At, &Number);

	if (Read == CALC_NUMBER_OVERFLOW) {
		strcpy(Str, "OVERFLOW");
		return;
	}

	if (Read != CALC_NUMBER_OK)
		return;

	long long Result = Number;

	for (;;) {
		char Operation = *At;

		if (Operation != '+' && Operation != '-' && Operation != '*'
			&& Operation != '/')
			break;

		At++;

		/* An operator with nothing after it -- "5+" -- leaves the display
		   holding what has been worked out so far. */
		Read = CalcNumber(&At, &Number);

		if (Read == CALC_NUMBER_OVERFLOW) {
			strcpy(Str, "OVERFLOW");
			return;
		}

		if (Read != CALC_NUMBER_OK)
			break;

		long long Stepped = 0;
		BOOL Over = FALSE;

		switch (Operation) {
		case '+':
			Over = __builtin_add_overflow(Result, Number, &Stepped);
			break;
		case '-':
			Over = __builtin_sub_overflow(Result, Number, &Stepped);
			break;
		case '*':
			Over = __builtin_mul_overflow(Result, Number, &Stepped);
			break;
		case '/':
			if (!Number) {
				strcpy(Str, "DIV0");
				return;
			}

			/* The one division with no answer in any width the machine
			   holds, which the processor would raise as a fault. */
			Over = (Result == (-9223372036854775807LL - 1) && Number == -1);

			if (!Over)
				Stepped = Result / Number;
			break;
		}

		if (Over) {
			strcpy(Str, "OVERFLOW");
			return;
		}

		Result = Stepped;
	}

	sprintf(Str, "%lld", Result);
}

/* -- The keys -- */

/*
 One key, from a button or from the keyboard. A word in the display -- DIV0,
 OVERFLOW -- is not something to go on from, and an answer is gone on from
 only by an operator: a digit starts the next sum.
 */
static void CalcPress(char Key) {
	char Typed[2] = { Key, 0x00 };

	char First = CalcCtx->DisplayStr[0];
	BOOL Word = (First && First != '-' && (First < '0' || First > '9'));
	BOOL Digit = (Key >= '0' && Key <= '9');

	/* An answer was not typed, so there is nothing of it to take back. */
	if (Key == CALC_BACK && CalcCtx->Answered && !Word)
		return;

	if (Word || (CalcCtx->Answered && Digit))
		CalcCtx->DisplayStr[0] = 0x00;

	CalcCtx->Answered = FALSE;

	switch (Key) {
	case 'C':
		CalcCtx->DisplayStr[0] = 0x00;
		break;

	case CALC_BACK: {
		size_t Length = strlen(CalcCtx->DisplayStr);

		if (Length)
			CalcCtx->DisplayStr[Length - 1] = 0x00;
		break;
	}

	case '=':
		CalcEvaluate(CalcCtx->DisplayStr);
		CalcCtx->Answered = TRUE;
		break;

	default:
		/* Bounded where the typing happens, so the buffer cannot fill. */
		if (strlen(CalcCtx->DisplayStr) < CALC_DIGITS)
			strcat(CalcCtx->DisplayStr, Typed);
		break;
	}

	CmnCtl2_SetText(CalcCtx->Display, CalcCtx->DisplayStr);
}

/*
 The keyboard, onto the same sixteen keys. Digits and + - * / = arrive as
 characters, from the main keys with Shift where they need it and from the
 keypad, and x is a times; Return and the keypad's Enter are =, Escape and
 Delete are C, and Backspace takes back the last thing typed. Zero for
 anything else, which is left to the controls.
 */
static char CalcKeyFor(WORD32 Message, WORD64 Param1) {
	if (Message == MSG_CHAR) {
		char Character = (char)Param1;

		if (Character >= '0' && Character <= '9')
			return Character;

		switch (Character) {
		case '+': case '-': case '*': case '/': case '=':
			return Character;
		case 'x': case 'X':
			return '*';
		case 'c': case 'C':
			return 'C';
		}

		return 0;
	}

	if (Message == MSG_KEYDOWN) {
		switch ((WORD32)Param1) {
		case 13:  return '=';           /* Return, and the keypad's Enter */
		case 27:  return 'C';           /* Escape */
		case 46:  return 'C';           /* Delete */
		case 8:   return CALC_BACK;     /* Backspace */
		}
	}

	return 0;
}

/* -- The window -- */

/* The display and the keypad, made once. The controls belong to the window
   and are painted, clicked and destroyed through it. */
static void Build(HANDLE Window) {
	CalcCtx->Display = CmnCtl2_CreateLabel(Window, 10, 40, "");

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			char Label[2] = { KeyPad[(y * 4) + x], 0x00 };

			CalcCtx->Keys[(y * 4) + x] = CmnCtl2_CreateButton(Window,
				10 + (x * (CALC_KEYW + CALC_GAP)),
				65 + (y * (CALC_KEYH + CALC_GAP)),
				CALC_KEYW, CALC_KEYH, Label);
		}
	}

	CalcCtx->Built = TRUE;
}

/*
 Everything that happens to the window, one message at a time. The answer is
 whether the message was used: a key taken here is not also given to whichever
 button last had the focus.
 */
__declspec(dllexport) WORD64 __WindowProcedure2(HANDLE Window, WORD32 Message,
	WORD64 Param1, WORD64 Param2
) {
	if (!CalcCtx)
		return 0;

	switch (Message) {
	case MSG_INIT:
		Build(Window);
		return 0;

	case MSG_PAINT:
		if (CalcCtx->Built)
			CmnCtl2_Paint(Window);
		return 0;

	case MSG_QUIT:
		/* The window manager closes the window once this returns, and the
		   controls go with it only if they are taken down here. */
		CmnCtl2_DestroyControls(Window);
		CalcCtx->Built = FALSE;
		return 0;
	}

	if (!CalcCtx->Built)
		return 0;

	char Key = CalcKeyFor(Message, Param1);

	if (Key) {
		CalcPress(Key);
		return 1;
	}

	/* The controls see everything else, and say afterwards which button was
	   pressed this time. */
	WORD64 Consumed = CmnCtl2_Dispatch(Window, Message, Param1, Param2);

	for (int i = 0; i < 16; i++) {
		if (CmnCtl2_WasClicked(CalcCtx->Keys[i]))
			CalcPress(KeyPad[i]);
	}

	return Consumed;
}
