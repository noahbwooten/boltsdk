#ifndef _STDDEF_H
#define _STDDEF_H

#ifdef __cplusplus
extern "C" {
#endif

	/* Define NULL as a pointer constant. */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

/* size_t must express the size of any object and ptrdiff_t any pointer
   difference, so both are pointer-width. As unsigned long and long they were
   right on x86 and collided with the compiler's own size_t on x64. */
#ifdef __x86_64__
	typedef unsigned long long size_t;
	typedef long long ptrdiff_t;
#else
	typedef unsigned long size_t;
	typedef long ptrdiff_t;
#endif

	/* Define wchar_t if not already defined. */
#ifndef __cplusplus
#ifndef _WCHAR_T
#define _WCHAR_T
	typedef unsigned short wchar_t;
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDDEF_H */
