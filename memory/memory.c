#include "memory.h"

int memcmp(const void *s1, const void *s2, size_t count) {
    const unsigned char *a = s1;
    const unsigned char *b = s2;
    for (size_t i = 0; i < count; i++) {
        if (a[i] != b[i]) return a[i] - b[i];
    }
    return 0;
}

void* memset32(void* dest, uint32_t val, size_t count) {
    uint8_t *d8 = (uint8_t *)dest;

    while (((uintptr_t)d8 & 7) && count > 0) {
        *d8++ = (uint8_t)(val & 0xFF);
        count--;
        val = (val << 24) | (val >> 8);
    }

    uint64_t pattern = ((uint64_t)val << 32) | val;
    uint64_t *d64 = (uint64_t *)d8;
    while (count >= 128) {
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        *(d64++) = pattern;
        count -= 128;
    }

    while (count >= 8) {
        *d64++ = pattern;
        count -= 8;
    } 
    d8 = (uint8_t *)d64;
    if (count >= 4) {
        *((uint32_t *)d8) = val;
        d8 += 4;
        count -= 4;
    }
    if (count >= 2) {
        *((uint16_t *)d8) = (uint16_t)val;
        d8 += 2;
        count -= 2;
    }
    if (count == 1)
        *d8 = val & 0xFF;

    return dest;
}

void* memset(void* dest, int byte, size_t count) {
    uint8_t b = (uint8_t)byte;
    uint8_t *d8 = (uint8_t *)dest;

    while (((uintptr_t)d8 & 7) && count > 0) {
        *d8++ = b;
        count--;
    }

    if (count >= 8) {
        uint64_t pattern = b;
        pattern |= pattern << 8;
        pattern |= pattern << 16;
        pattern |= pattern << 32;

        uint64_t *d64 = (uint64_t*)d8;
        while (count >= 64) {
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            *d64++ = pattern;
            count -= 64;
        }

        while (count >= 8) {
            *d64++ = pattern;
            count -= 8;
        }
        d8 = (uint8_t*)d64;
    }

    while (count--) {
        *d8++ = b;
    }

    return dest;
}

void* memcpy(void *restrict dest, const void *restrict src, size_t count) {
    uint8_t *d8 = dest;
    const uint8_t *s8 = src;

    if (count == 0 || dest == src) return dest;
    if ((((uintptr_t)d8 ^ (uintptr_t)s8) & 7) == 0) {
        while (((uintptr_t)d8 & 7) && count > 0) {
            *d8++ = *s8++;
            count--;
        }

        uint64_t *d64 = (uint64_t *)d8;
        const uint64_t *s64 = (const uint64_t *)s8;
        while (count >= 128) {
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            count -= 128;
        }

        while (count >= 32) {
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            count -= 32;
        }

        while (count >= 8) {
            *d64++ = *s64++;
            count -= 8;
        }
        d8 = (uint8_t *)d64;
        s8 = (const uint8_t *)s64;
    }

    while (count >= 16) {
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        *d8++ = *s8++;
        count -= 16;
    }
    while (count--) *d8++ = *s8++;

    return dest;
}

void memreverse(void *ptr, size_t n) {
    if (n <= 1) return;

    uint8_t *l = (uint8_t*)ptr;
    uint8_t *r = l + n - 1;

    while (((uintptr_t)l & 7) &&l < r) {
        uint8_t t = *l;
        *l++ = *r;
        *r-- = t;
    }

    while (((uintptr_t)r & 7) && l < r) {
        uint8_t t = *l;
        *l++ = *r;
        *r-- = t;
    }

    while ((size_t)(r - l + 1) >= 16) {
        uint64_t *pl = (uint64_t*)l;
        uint64_t *pr = (uint64_t*)(r-7);

        uint64_t vl = *pl;
        uint64_t vr = *pr;

        *pl = bswap64(vr);
        *pr = bswap64(vl);

        l += 8;
        r -= 8;
    }

    while (l < r) {
        uint8_t t = *l;
        *l++ = *r;
        *r-- = t;
    }
}
void* memmove(void *dest, const void *src, size_t count) {
    if (dest == src || count == 0) return dest;

    uint8_t *d8 = (uint8_t *)dest;
    const uint8_t *s8 = (const uint8_t *)src;

    uintptr_t d = (uintptr_t)d8;
    uintptr_t s = (uintptr_t)s8;
    if (d < s || d - s >= count) return memcpy(dest, src, count);
    d8 += count;
    s8 += count;

    if ((((uintptr_t)d8 ^(uintptr_t)s8) & 7) != 0) {
        while (count >= 16) {
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            *--d8 = *--s8;
            count -= 16;
        }

        while (count--) *--d8 = *--s8;
        return dest;
    }

    while (count > 0 && ((uintptr_t)d8 & 7)) {
        *--d8 = *--s8;
        count--;
    }

    uint64_t *d64 = (uint64_t *)d8;
    const uint64_t *s64 = (const uint64_t *)s8;

    while (count >= 128) {
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        count -= 128;
    }

    while (count >= 32) {
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        *--d64 = *--s64;
        count -= 32;
    }

    while (count >= 8) {
        *--d64 = *--s64;
        count -= 8;
    }

    d8 = (uint8_t *)d64;
    s8 = (const uint8_t *)s64;

    while (count--) *--d8 = *--s8;
    return dest;
}

void* memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
	if(!haystack || !needle) return 0;
	if(!needle_len) return (void*)haystack;
	if(haystack_len < needle_len) return 0;

	const uint8_t* h = haystack;
	const uint8_t* n = needle;

	if (haystack_len == 1) {
        uint8_t c = n[0];
        for (size_t i = 0; i < haystack_len; i++) if (h[i] == c) return (void*)(h + i);
        return 0;
    }

	size_t last = haystack_len - needle_len;
    if (needle_len == 2) {
        uint8_t c0 = n[0];
        uint8_t c1 = n[1];
        for(size_t i = 0; i <= last; i++) if (h[i] == c0 && h[i+1] == c1) return (void*)(h + i);
        return 0;
    }

	if (haystack_len < 128 || needle_len < 4) {
        uint8_t first = n[0];
        uint8_t final = n[needle_len-1];

	    for(size_t i = 0; i <= last; i++) {
		    if (h[i] != first) continue;
            if (h[i + needle_len - 1] != final) continue;
		    if (!memcmp(h + i+1, n+1, needle_len-2)) return (void*)(h+i);
        }

        return 0;
    }

    if (needle_len <= 255) {
        uint8_t skip[256];

        for (size_t i = 0; i < 256; i++) skip[i] = (uint8_t)needle_len;
        for (size_t i = 0; i < needle_len - 1; i++) skip[n[i]] = (uint8_t)(needle_len - 1 - i);

        size_t i = 0;
        uint8_t final = n[needle_len-1];

        while (i <= last) {
            uint8_t c = h[i + needle_len-1];
            if (c == final && !memcmp(h + i, n, needle_len - 1)) return (void*)(h+i);
            i += skip[c];
        }

	    return 0;
	}

    size_t skip[256];
    for (size_t i = 0; i < 256; i++) skip[i] = needle_len;
    for (size_t i = 0; i < needle_len - 1; i++) skip[n[i]] = needle_len - 1 - i;
    size_t i = 0;
    uint8_t final = n[needle_len-1];

    while (i <= last) {
        uint8_t c = h[i + needle_len-1];
        if (c == final && !memcmp(h + i, n, needle_len - 1)) return (void*)(h+i);
        i += skip[c];
	}

	return 0;
}
