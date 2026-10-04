#ifndef ALL_STRING_H
#define ALL_STRING_H

#include "stddef.h"  // for size_t
#include "wchar.h"  // if wide-char functions are needed
#include "umbase.h"

UMI_FUNCTION void* memcpy(void* dest, const void* src, size_t n);
UMI_FUNCTION void* memmove(void* dest, const void* src, size_t n);
UMI_FUNCTION void* memset(void* s, int c, size_t n);
UMI_FUNCTION int memcmp(const void* s1, const void* s2, size_t n);

/* String Manipulation */
UMI_FUNCTION char* strcpy(char* dest, const char* src);
UMI_FUNCTION char* strncpy(char* dest, const char* src, size_t n);
UMI_FUNCTION char* strcat(char* dest, const char* src);

/* BSD Extension: Safe String Copy/Concatenation */
UMI_FUNCTION size_t strlcpy(char* dest, const char* src, size_t size);
UMI_FUNCTION size_t strlcat(char* dest, const char* src, size_t size);

/* String Comparison */
UMI_FUNCTION int strcmp(const char* s1, const char* s2);
UMI_FUNCTION int strncmp(const char* s1, const char* s2, size_t n);
UMI_FUNCTION int strcoll(const char* s1, const char* s2);
UMI_FUNCTION size_t strxfrm(char* dest, const char* src, size_t n);

/* String Length & Search */
UMI_FUNCTION size_t strlen(const char* s);
UMI_FUNCTION char* strchr(const char* s, int c);
UMI_FUNCTION char* strrchr(const char* s, int c);
UMI_FUNCTION char* strstr(const char* haystack, const char* needle);
UMI_FUNCTION char* strpbrk(const char* s, const char* accept);
UMI_FUNCTION size_t strspn(const char* s, const char* accept);
UMI_FUNCTION size_t strcspn(const char* s, const char* reject);

/* String Tokenization */
UMI_FUNCTION char* strtok(char* s, const char* delim);
/* Note: strtok_r is POSIX and not part of the C standard */
UMI_FUNCTION char* strtok_r(char* s, const char* delim, char** saveptr);

UMI_FUNCTION char* strdup(char* s);

UMI_FUNCTION void* memchr(const void* ptr, int ch, size_t count);


#endif /* ALL_STRING_H */
