#pragma once
/*
stdarg.h
BoltOS Usermode Include Declarations
Standard Variardic Arguments

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

typedef char* va_list;

#ifdef __x86_64__

/*
 The x86 definitions below cannot work on x64: the first four arguments arrive
 in registers and are spilled only when __va_start marks the function variadic,
 every slot is 8 bytes wide, and anything larger is passed as a pointer.
 */

#define va_start(ap, last)  ((void)__va_start(&ap, last))

#define _VA_SLOT(ap, type)  ((ap) += 8, (ap) - 8)
#define _VA_INDIRECT(type)  ((sizeof(type) > 8) || ((sizeof(type) & (sizeof(type) - 1)) != 0))

#define va_arg(ap, type)    (_VA_INDIRECT(type) \
                                ? **(type**)_VA_SLOT(ap, type) \
                                :  *(type*) _VA_SLOT(ap, type))

#define va_end(ap)          ((void)((ap) = 0))

#else

// x86 cdecl: arguments are pushed contiguously, so walking the stack directly
// is valid.
#define va_start(ap, last)   ((ap) = (char*)&last + sizeof(last))
#define va_arg(ap, type)      (*(type*)((ap) += sizeof(type), (ap) - sizeof(type)))
#define va_end(ap)            ((void)0)

#endif
