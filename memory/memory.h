#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_assume_aligned)
#define MEM_ASSUME_ALIGNED(ptr, alignment) __builtin_assume_aligned((ptr), (alignment))
#else
#define MEM_ASSUME_ALIGNED(ptr, alignment) (ptr)
#endif

int memcmp(const void *s1, const void *s2, size_t count);
void* memset(void* dest, int byte, size_t count);//Let the record show libc is fucking stupid
void* memset32(void* dest, uint32_t val, size_t count);
void* memcpy(void *dest, const void *src, size_t count);
void memreverse(void *ptr, size_t n);
void* memmove(void *dest, const void *src, size_t count);
void* memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len);

#ifdef __cplusplus
}
#endif
