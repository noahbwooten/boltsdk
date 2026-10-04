#ifndef _WCHAR_H
#define _WCHAR_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "stddef.h"  // for size_t
#include "boltsdk.h"

	/* Define wchar_t if not already defined */
#ifndef __cplusplus
#ifndef _WCHAR_T
#define _WCHAR_T
	typedef unsigned short wchar_t;
#endif
#endif

	/*
	 Wide character functions. The kernel has them and no module exports
	 them, so an application is given the type and not the calls: declared
	 only where BOLTSDK_APIV is not, which is the system building itself.
	 */

#ifdef __cplusplus
}
#endif

#endif /* _WCHAR_H */
