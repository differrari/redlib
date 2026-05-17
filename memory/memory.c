#include "memory.h"

int memcmp(const void *s1, const void *s2, size_t count) {
    const uint8_t *a = s1;
    const uint8_t *b = s2;

    if ((((uintptr_t)a ^ (uintptr_t)b) & 15) == 0) {
        while (((uintptr_t)a & 15) && count > 0) {
            if (*a != *b) return *a - *b;
            a++;
            b++;
            count--;
        }

        const uint128_t *a128 = (const uint128_t*)a;
        const uint128_t *b128 = (const uint128_t*)b;
        while (count >= 32) {
            uint128_t va0 = a128[0];
            uint128_t vb0 = b128[0];
            uint128_t va1 = a128[1];
            uint128_t vb1 = b128[1];

            if (va0 != vb0) {
                a = (const uint8_t*)a128;
                b = (const uint8_t*)b128;
                for (size_t i = 0; i < 16; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            if (va1 != vb1) {
                a = (const uint8_t*)(a128 + 1);
                b = (const uint8_t*)(b128 + 1);
                for (size_t i = 0; i < 16; i++) if (a[i] != b[i]) return a[i] - b[i];
            }

            a128 += 2;
            b128 += 2;
            count -= 32;
        }

        while (count >= 16) {
            uint128_t va = *a128++;
            uint128_t vb = *b128++;
            if (va != vb) {
                a = (const uint8_t*)(a128 - 1);
                b = (const uint8_t*)(b128 - 1);
                for (size_t i = 0; i < 16; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            count -= 16;
        }

        a = (const uint8_t*)a128;
        b = (const uint8_t*)b128;
    } else if ((((uintptr_t)a ^ (uintptr_t)b) & 7) == 0) {
        while (((uintptr_t)a & 7) && count > 0) {
            if (*a != *b) return *a - *b;
            a++;
            b++;
            count--;
        }

        const uint64_t *a64 = (const uint64_t*)a;
        const uint64_t *b64 = (const uint64_t*)b;
        while (count >= 32) {
            uint64_t va0 = a64[0];
            uint64_t vb0 = b64[0];
            uint64_t va1 = a64[1];
            uint64_t vb1 = b64[1];
            uint64_t va2 = a64[2];
            uint64_t vb2 = b64[2];
            uint64_t va3 = a64[3];
            uint64_t vb3 = b64[3];

            if (va0 != vb0) {
                a = (const uint8_t*)a64;
                b = (const uint8_t*)b64;
                for (size_t i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            if (va1 != vb1) {
                a = (const uint8_t*)(a64 + 1);
                b = (const uint8_t*)(b64 + 1);
                for (size_t i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            if (va2 != vb2) {
                a = (const uint8_t*)(a64 + 2);
                b = (const uint8_t*)(b64 + 2);
                for (size_t i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            if (va3 != vb3) {
                a = (const uint8_t*)(a64 + 3);
                b = (const uint8_t*)(b64 + 3);
                for (size_t i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] - b[i];
            }

            a64 += 4;
            b64 += 4;
            count -= 32;
        }

        while (count >= 8) {
            uint64_t va = *a64++;
            uint64_t vb = *b64++;
            if (va != vb) {
                a = (const uint8_t*)(a64 - 1);
                b = (const uint8_t*)(b64 - 1);
                for (size_t i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] - b[i];
            }
            count -= 8;
        }

        a = (const uint8_t*)a64;
        b = (const uint8_t*)b64;
    }

    while (count > 0) {
        if (*a != *b) return *a - *b;
        a++;
        b++;
        count--;
    }
    return 0;
}

void* memset32(void* dest, uint32_t val, size_t count) {
    uint8_t *d8 = (uint8_t*)dest;

    while (((uintptr_t)d8 & 15) && count > 0) {
        *d8++ = (uint8_t)(val & 0xFF);
        count--;
        val = (val >> 8) | (val << 24);
    }

    uint64_t pattern = ((uint64_t)val << 32) | val;
    if (count >= 16) {
        uint128_t pattern128 = ((uint128_t)pattern << 64) | pattern;
        uint128_t *d128 = (uint128_t*)__builtin_assume_aligned(d8, 16);

        while (count >= 128) {
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            count -= 128;
        }

        while (count >= 32) {
            *d128++ = pattern128;
            *d128++ = pattern128;
            count -= 32;
        }

        while (count >= 16) {
            *d128++ = pattern128;
            count -= 16;
        }

        d8 = (uint8_t*)d128;
    }

    if (count >= 8) {
        uint64_t *d64 = (uint64_t*)__builtin_assume_aligned(d8, 8);
        *d64++ = pattern;
        d8 = (uint8_t*)d64;
        count -= 8;
    } 
    if (count >= 4) {
        *((uint32_t*)d8) = val;
        d8 += 4;
        count -= 4;
    }
    while (count > 0) {
        *d8++ = (uint8_t)(val & 0xFF);
        val = (val >> 8) | (val << 24);
        count--;
    }

    return dest;
}

void* memset(void* dest, int byte, size_t count) {
    uint8_t b = (uint8_t)byte;
    uint8_t *d8 = (uint8_t*)dest;

    while (((uintptr_t)d8 & 15) && count > 0) {
        *d8++ = b;
        count--;
    }

    uint64_t pattern = b;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    pattern |= pattern << 32;

    if (count >= 16) {
        uint128_t pattern128 = ((uint128_t)pattern << 64) | pattern;
        uint128_t *d128 = (uint128_t*)__builtin_assume_aligned(d8, 16);
        while (count >= 128) {
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            *d128++ = pattern128;
            count -= 128;
        }

        while (count >= 32) {
            *d128++ = pattern128;
            *d128++ = pattern128;
            count -= 32;
        }

        while (count >= 16) {
            *d128++ = pattern128;
            count -= 16;
        }

        d8 = (uint8_t*)d128;
    }

    if (count >= 8) {
        uint64_t *d64 = (uint64_t*)__builtin_assume_aligned(d8, 8);
        *d64++ = pattern;
        d8 = (uint8_t*)d64;
        count -= 8;
    }

    while (count--) {
        *d8++ = b;
    }

    return dest;
}

void* memcpy(void *restrict dest, const void *restrict src, size_t count) {
    if (count == 0 || dest == src) return dest;
    uint8_t *d8 = (uint8_t*)dest;
    const uint8_t *s8 = (const uint8_t*)src;
    if ((((uintptr_t)d8 ^ (uintptr_t)s8) & 15) == 0) {
        while (((uintptr_t)d8 & 15) && count > 0) {
            *d8++ = *s8++;
            count--;
        }

        if (count >= 16) {
            uint128_t *d128 = (uint128_t*)__builtin_assume_aligned(d8, 16);
            const uint128_t *s128 = (const uint128_t*)__builtin_assume_aligned(s8, 16);
            while (count >= 128) {
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                *d128++ = *s128++;
                count -= 128;
            }

            while (count >= 32) {
                *d128++ = *s128++;
                *d128++ = *s128++;
                count -= 32;
            }

            while (count >= 16) {
                *d128++ = *s128++;
                count -= 16;
            }

            d8 = (uint8_t*)d128;
            s8 = (const uint8_t*)s128;
        }
    }

    while (((uintptr_t)d8 & 7) && count > 0) {
        *d8++ = *s8++;
        count--;
    }

    if (count >= 8) {
        uint64_t *d64 = (uint64_t*)__builtin_assume_aligned(d8, 8);
        if (((uintptr_t)s8 & 7) == 0) {
            const uint64_t *s64 = (const uint64_t*)__builtin_assume_aligned(s8, 8);
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
            d8 = (uint8_t*)d64;
            s8 = (const uint8_t*)s64;
        } else if (count >= 16) {
            size_t shift = ((uintptr_t)s8 & 7) * 8;
            size_t rshift = 64 - shift;
            size_t overread = 16 - (shift >> 3);
            const uint64_t *s64 = (const uint64_t*)((uintptr_t)s8 & ~(uintptr_t)7);
            uint64_t lo = *s64++;

            while (count >= overread + 24) {
                uint64_t hi0 = *s64++;
                uint64_t hi1 = *s64++;
                uint64_t hi2 = *s64++;
                uint64_t hi3 = *s64++;

                *d64++ = (lo >> shift) | (hi0 << rshift);
                *d64++ = (hi0 >> shift) | (hi1 << rshift);
                *d64++ = (hi1 >> shift) | (hi2 << rshift);
                *d64++ = (hi2 >> shift) | (hi3 << rshift);

                lo = hi3;
                s8 += 32;
                count -= 32;
            }

            while (count >= overread) {
                uint64_t hi = *s64++;
                *d64++ = (lo >> shift) | (hi << rshift);
                lo = hi;
                s8 += 8;
                count -= 8;
            }

            d8 = (uint8_t*)d64;
        }
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
    size_t left = (uintptr_t)l & 15;
    size_t right = (uintptr_t)(r+1) & 15;
    size_t align_left = (16 - left) & 15;

    if (right == align_left) {
        while (align_left-- && l < r) {
            uint8_t t = *l;
            *l++ = *r;
            *r-- = t;
        }

        while ((size_t)(r - l + 1) >= 32) {
            uint128_t *pl = (uint128_t*)l;
            uint128_t *pr = (uint128_t*)(r - 15);

            uint128_t vl = *pl;
            uint128_t vr = *pr;

            *pl = bswap128(vr);
            *pr = bswap128(vl);

            l += 16;
            r -= 16;
        }
    }

    left = (uintptr_t)l & 7;
    right = (uintptr_t)(r+1) & 7;
    align_left = (8 - left) & 7;

    if (right == align_left) {
        while (align_left-- && l < r) {
            uint8_t t = *l;
            *l++ = *r;
            *r-- = t;
        }

        while ((size_t)(r - l + 1) >= 32) {
            uint64_t *pl1 = (uint64_t*)l;
            uint64_t *pl2 = (uint64_t*)(l + 8);
            uint64_t *pr1 = (uint64_t*)(r - 15);
            uint64_t *pr2 = (uint64_t*)(r-7);

            uint64_t vl1 = *pl1;
            uint64_t vl2 = *pl2;
            uint64_t vr1 = *pr1;
            uint64_t vr2 = *pr2;

            *pl1 = bswap64(vr2);
            *pl2 = bswap64(vr1);
            *pr1 = bswap64(vl2);
            *pr2 = bswap64(vl1);

            l += 16;
            r -= 16;
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
    } else if (align_left <= right) {
        while (align_left-- && l< r) {
            uint8_t t = *l;
            *l++ = *r;
            *r-- = t;
        }

        while ((size_t)(r - l + 1) >= 32) {
            uint64_t *pl1 = (uint64_t*)l;
            uint64_t *pl2 = (uint64_t*)(l + 8);

            uint64_t vl1 = *pl1;
            uint64_t vl2 = *pl2;
            uint64_t vr1 = (uint64_t)r[0] | ((uint64_t)r[-1] << 8) | ((uint64_t)r[-2] << 16) | ((uint64_t)r[-3] << 24) | ((uint64_t)r[-4] << 32) | ((uint64_t)r[-5] << 40) | ((uint64_t)r[-6] << 48) | ((uint64_t)r[-7] << 56);
            uint64_t vr2 = (uint64_t)r[-8] | ((uint64_t)r[-9] << 8) | ((uint64_t)r[-10] << 16) | ((uint64_t)r[-11] << 24) | ((uint64_t)r[-12] << 32) | ((uint64_t)r[-13] << 40) | ((uint64_t)r[-14] << 48) | ((uint64_t)r[-15] << 56);
            uint64_t out1 = bswap64(vl1);
            uint64_t out2 = bswap64(vl2);

            *pl1 = vr1;
            *pl2 = vr2;
            r[-15] = (uint8_t)out2;
            r[-14] = (uint8_t)(out2 >> 8);
            r[-13] = (uint8_t)(out2 >> 16);
            r[-12] = (uint8_t)(out2 >> 24);
            r[-11] = (uint8_t)(out2 >> 32);
            r[-10] = (uint8_t)(out2 >> 40);
            r[-9] = (uint8_t)(out2 >> 48);
            r[-8] = (uint8_t)(out2 >> 56);
            r[-7] = (uint8_t)out1;
            r[-6] = (uint8_t)(out1 >> 8);
            r[-5] = (uint8_t)(out1 >> 16);
            r[-4] = (uint8_t)(out1 >> 24);
            r[-3] = (uint8_t)(out1 >> 32);
            r[-2] = (uint8_t)(out1 >> 40);
            r[-1] = (uint8_t)(out1 >> 48);
            r[0] = (uint8_t)(out1 >> 56);

            l += 16;
            r -= 16;
        }

        while ((size_t)(r - l + 1) >= 16) {
            uint64_t vl = *(uint64_t*)l;
            uint64_t vr = (uint64_t)r[0] | ((uint64_t)r[-1] << 8) | ((uint64_t)r[-2] << 16) | ((uint64_t)r[-3] << 24) | ((uint64_t)r[-4] << 32) | ((uint64_t)r[-5] << 40) | ((uint64_t)r[-6] << 48) | ((uint64_t)r[-7] << 56);
            uint64_t out = bswap64(vl);

            *(uint64_t*)l = vr;
            r[-7] = (uint8_t)out;
            r[-6] = (uint8_t)(out >> 8);
            r[-5] = (uint8_t)(out >> 16);
            r[-4] = (uint8_t)(out >> 24);
            r[-3] = (uint8_t)(out >> 32);
            r[-2] = (uint8_t)(out >> 40);
            r[-1] = (uint8_t)(out >> 48);
            r[0] = (uint8_t)(out >> 56);

            l += 8;
            r -= 8;
        }
    } else {
        size_t align_right = right;
        while (align_right-- && l < r) {
            uint8_t t = *l;
            *l++ = *r;
            *r-- = t;
        }

        while ((size_t)(r - l+1) >= 32) {
            uint64_t *pr1 = (uint64_t*)(r - 15);
            uint64_t *pr2 = (uint64_t*)(r - 7);

            uint64_t vr1 = *pr1;
            uint64_t vr2 = *pr2;
            uint64_t vl1 = (uint64_t)l[7] | ((uint64_t)l[6] << 8) | ((uint64_t)l[5] << 16) | ((uint64_t)l[4] << 24) | ((uint64_t)l[3] << 32) | ((uint64_t)l[2] << 40) | ((uint64_t)l[1] << 48) | ((uint64_t)l[0] << 56);
            uint64_t vl2 = (uint64_t)l[15] | ((uint64_t)l[14] << 8) | ((uint64_t)l[13] << 16) | ((uint64_t)l[12] << 24) | ((uint64_t)l[11] << 32) | ((uint64_t)l[10] << 40) | ((uint64_t)l[9] << 48) | ((uint64_t)l[8] << 56);
            uint64_t out1 = bswap64(vr1);
            uint64_t out2 = bswap64(vr2);

            l[0] = (uint8_t)out2;
            l[1] = (uint8_t)(out2 >> 8);
            l[2] = (uint8_t)(out2 >> 16);
            l[3] = (uint8_t)(out2 >> 24);
            l[4] = (uint8_t)(out2 >> 32);
            l[5] = (uint8_t)(out2 >> 40);
            l[6] = (uint8_t)(out2 >> 48);
            l[7] = (uint8_t)(out2 >> 56);
            l[8] = (uint8_t)out1;
            l[9] = (uint8_t)(out1 >> 8);
            l[10] = (uint8_t)(out1 >> 16);
            l[11] = (uint8_t)(out1 >> 24);
            l[12] = (uint8_t)(out1 >> 32);
            l[13] = (uint8_t)(out1 >> 40);
            l[14] = (uint8_t)(out1 >> 48);
            l[15] = (uint8_t)(out1 >> 56);
            *pr1 = vl2;
            *pr2 = vl1;

            l += 16;
            r -= 16;
        }

        while ((size_t)(r - l+1) >= 16) {
            uint64_t *pr = (uint64_t*)(r - 7);
            uint64_t vr = *pr;
            uint64_t vl = (uint64_t)l[7] | ((uint64_t)l[6] << 8) | ((uint64_t)l[5] << 16) | ((uint64_t)l[4] << 24) | ((uint64_t)l[3] << 32) | ((uint64_t)l[2] << 40) | ((uint64_t)l[1] << 48) | ((uint64_t)l[0] << 56);
            uint64_t out = bswap64(vr);

            l[0] = (uint8_t)out;
            l[1] = (uint8_t)(out >> 8);
            l[2] = (uint8_t)(out >> 16);
            l[3] = (uint8_t)(out >> 24);
            l[4] = (uint8_t)(out >> 32);
            l[5] = (uint8_t)(out >> 40);
            l[6] = (uint8_t)(out >> 48);
            l[7] = (uint8_t)(out >> 56);
            *pr = vl;

            l += 8;
            r -= 8;
        }
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
    const uint8_t *s8 = (const uint8_t*)src;

    uintptr_t d = (uintptr_t)d8;
    uintptr_t s = (uintptr_t)s8;
    if (d < s) {
        if (s - d >= count) return memcpy(dest, src, count);

        while (((uintptr_t)d8 & 7) && count > 0) {
            *d8++ = *s8++;
            count--;
        }

        uint64_t *d64 = (uint64_t*)d8;
        if (((uintptr_t)s8 & 7) == 0) {
            const uint64_t *s64 = (const uint64_t*)s8;
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

            d8 = (uint8_t*)d64;
            s8 = (const uint8_t*)s64;
        } else if (count >= 16) {
            size_t shift = ((uintptr_t)s8 & 7) * 8;
            size_t rshift = 64 - shift;
            size_t overread = 16 - (shift >> 3);
            const uint64_t *s64 = (const uint64_t*)((uintptr_t)s8 & ~(uintptr_t)7);
            uint64_t lo = *s64++;

            while (count >= overread + 56) {
                uint64_t hi0 = *s64++;
                uint64_t hi1 = *s64++;
                uint64_t hi2 = *s64++;
                uint64_t hi3 = *s64++;
                uint64_t hi4 = *s64++;
                uint64_t hi5 = *s64++;
                uint64_t hi6 = *s64++;
                uint64_t hi7 = *s64++;

                *d64++ = (lo >> shift) | (hi0 << rshift);
                *d64++ = (hi0 >> shift) | (hi1 << rshift);
                *d64++ = (hi1 >> shift) | (hi2 << rshift);
                *d64++ = (hi2 >> shift) | (hi3 << rshift);
                *d64++ = (hi3 >> shift) | (hi4 << rshift);
                *d64++ = (hi4 >> shift) | (hi5 << rshift);
                *d64++ = (hi5 >> shift) | (hi6 << rshift);
                *d64++ = (hi6 >> shift) | (hi7 << rshift);

                lo = hi7;
                s8 += 64;
                count -= 64;
            }

            while (count >= overread + 24) {
                uint64_t hi0 = *s64++;
                uint64_t hi1 = *s64++;
                uint64_t hi2 = *s64++;
                uint64_t hi3 = *s64++;

                *d64++ = (lo >> shift) | (hi0 << rshift);
                *d64++ = (hi0 >> shift) | (hi1 << rshift);
                *d64++ = (hi1 >> shift) | (hi2 << rshift);
                *d64++ = (hi2 >> shift) | (hi3 << rshift);

                lo = hi3;
                s8 += 32;
                count -= 32;
            }

            while (count >= overread) {
                uint64_t hi = *s64++;
                *d64++ = (lo >> shift) | (hi << rshift);
                lo = hi;
                s8 += 8;
                count -= 8;
            }

            d8 = (uint8_t*)d64;
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

    if (d - s >= count) return memcpy(dest, src, count);
    d8 += count;
    s8 += count;

    while (((uintptr_t)d8 & 7) && count > 0) {
        *--d8 = *--s8;
        count--;
    }

    uint64_t *d64 = (uint64_t *)d8;
    if (((uintptr_t)s8 & 7) == 0) {
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
        s8 = (const uint8_t*)s64;
    } else if (count >= 8) {
        const uint8_t *p = s8 - 8;
        size_t shift = ((uintptr_t)p & 7) * 8;
        size_t rshift = 64 - shift;
        const uint64_t *s64 = (const uint64_t*)((uintptr_t)p & ~(uintptr_t)7);

        while (count >= 64) {
            uint64_t v0 = (s64[0] >> shift) | (s64[1] << rshift);
            uint64_t v1 = (s64[-1] >> shift) | (s64[0] << rshift);
            uint64_t v2 = (s64[-2] >> shift) | (s64[-1] << rshift);
            uint64_t v3 = (s64[-3] >> shift) | (s64[-2] << rshift);
            uint64_t v4 = (s64[-4] >> shift) | (s64[-3] << rshift);
            uint64_t v5 = (s64[-5] >> shift) | (s64[-4] << rshift);
            uint64_t v6 = (s64[-6] >> shift) | (s64[-5] << rshift);
            uint64_t v7 = (s64[-7] >> shift) | (s64[-6] << rshift);

            *--d64 = v0;
            *--d64 = v1;
            *--d64 = v2;
            *--d64 = v3;
            *--d64 = v4;
            *--d64 = v5;
            *--d64 = v6;
            *--d64 = v7;

            s64 -= 8;
            s8 -= 64;
            count -= 64;
        }

        while (count >= 32) {
            uint64_t v0 = (s64[0] >> shift) | (s64[1] << rshift);
            uint64_t v1 = (s64[-1] >> shift) | (s64[0] << rshift);
            uint64_t v2 = (s64[-2] >> shift) | (s64[-1] << rshift);
            uint64_t v3 = (s64[-3] >> shift) | (s64[-2] << rshift);

            *--d64 = v0;
            *--d64 = v1;
            *--d64 = v2;
            *--d64 = v3;

            s64 -= 4;
            s8 -= 32;
            count -= 32;
        }

        while (count >= 8) {
            uint64_t v = (s64[0] >> shift) | (s64[1] << rshift);
            *--d64 = v;
            s64--;
            s8 -= 8;
            count -= 8;
        }

        d8 = (uint8_t*)d64;
    }

    while (count--) *--d8 = *--s8;
    return dest;
}

void* memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len) {
	if(!haystack || !needle) return 0;
	if(!needle_len) return (void*)haystack;
	if(haystack_len < needle_len) return 0;

	const uint8_t* h = haystack;
	const uint8_t* n = needle;

	if (needle_len == 1) {
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
