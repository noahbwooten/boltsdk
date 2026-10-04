#pragma once
/*
umerr.h
BoltOS Usermode Include Declarations
User-Mode Error Definitons

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "boltsdk.h"

#if !defined(BOLTSDK_APIV) || (BOLTSDK_APIV >= BOLTSDK_APIV_10)

#define ERROR_SUCCESS            0x00 // Success.
#define ERROR_PATH_NOT_FOUND     0x01 // Path could not be opened.
#define ERROR_ACCESS_DENIED      0x02 // Access to the resource is denied.
#define ERROR_INVALID_HANDLE     0x03 // The provided handle is not valid.
#define ERROR_INVALID_MODE       0x04 // The system cannot accept this request in it's current state.
#define ERROR_MEMORY_FAILURE     0x05 // The memory allocation failed without explanation.
#define ERROR_OUT_OF_RESOURCES   0x06 // The memory allocation failed 
#define ERROR_INVALID_ARGUMENT   0x07 // The argument passed is not valid.
#define ERROR_INVALID_PATH       0x08 // Path does not make logical sense.
#define ERROR_INVALID_FILE_ID    0x09 // File ID specified is not valid
#define ERROR_INVALID_CSR_MODIF  0x0A // The specified cursor movement type is not valid.
#define ERROR_INVALID_SVC_NAME   0x0B // The specified service name does not exist.
#define ERROR_INVALID_SVC_ID     0x0C // The specified service ID does not exist.
#define ERROR_SVC_ALREADY_RUN    0x0D // The specified service is already running.
#define ERROR_SVC_ALREADY_STOP   0x0E // The specified service is already stopped.
#define ERROR_SVC_ALREADY_PAUSE  0x0F // The specified service is already paused.

#endif /* BOLTSDK_APIV_10 */
