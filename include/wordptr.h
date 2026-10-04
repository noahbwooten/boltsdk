	#pragma once
/*
 wordptr.h
 (c)Noah Wooten 2023 - 2026, All Rights Reserved

 An integer wide enough to hold a pointer. Anything storing an address in an
 integer must use this rather than WORD32, which is 32 bits on every target.
 */

#ifndef _BOLT_WORDPTR_DEFINED
#define _BOLT_WORDPTR_DEFINED

#ifdef __x86_64__
typedef unsigned long long WORDPTR, *PWORDPTR;
#else
typedef unsigned long WORDPTR, *PWORDPTR;
#endif

#endif
