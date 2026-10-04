#pragma once
/*
services.h
BoltOS Usermode Include Declarations
User-Mode Services Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define SVCMSG_INIT  0
#define SVCMSG_HALT  1
#define SVCMSG_NSCLK 2
#define SVCMSG_1SCLK 2

#endif /* BOLTSDK_APIV_10 */
