#pragma once
/*
stdio.h
BoltOS Usermode Include Declarations
Standard I/O Functions

(c)Noah Wooten 2023 - 2026, All Rights Reserved
*/

#include "umbase.h"
#include "stdarg.h"

UMI_FUNCTION int sprintf(char* buffer, const char* format, ...);
UMI_FUNCTION int vsprintf(char* buffer, const char* format, va_list args);
UMI_FUNCTION int vsscanf(const char* str, const char* format, va_list args);
UMI_FUNCTION int sscanf(const char* str, const char* format, ...);
UMI_FUNCTION void qsort(void* base, unsigned long num, unsigned long size, int (*cmp)(const void*, const void*));