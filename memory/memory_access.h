#pragma once

#include "types.h"

#define read8(addr) (*(volatile uint8_t*)(addr))
#define write8(addr, value) (*(volatile uint8_t*)(addr) = (value))

#define read16(addr) (*(volatile uint16_t*)(addr))
#define write16(addr, value) (*(volatile uint16_t*)(addr) = (value))

#define read32(addr) (*(volatile uint32_t*)(addr))
#define write32(addr, value) (*(volatile uint32_t*)(addr) = (value))

#define read64(addr) (*(volatile uint64_t*)(addr))
#define write64(addr, value) (*(volatile uint64_t*)(addr) = (value))

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t read_unaligned16(const void *up) {
    const volatile uint8_t *p = (const volatile uint8_t*)up;
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t read_unaligned32(const void *up) {
    const volatile uint8_t *p = (const volatile uint8_t*)up;
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline uint64_t read_unaligned64(const void *up) {
    const volatile uint8_t *p = (const volatile uint8_t*)up;
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
}

static inline void write_unaligned32(void *up, uint32_t value) {
    volatile uint8_t *p = (volatile uint8_t*)up;
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
    p[2] = (uint8_t)((value >> 16) & 0xFF);
    p[3] = (uint8_t)((value >> 24) & 0xFF);
}

static inline void write_unaligned64(void *up, uint64_t value) {
    volatile uint8_t *p = (volatile uint8_t*)up;
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
    p[2] = (uint8_t)((value >> 16) & 0xFF);
    p[3] = (uint8_t)((value >> 24) & 0xFF);
    p[4] = (uint8_t)((value >> 32) & 0xFF);
    p[5] = (uint8_t)((value >> 40) & 0xFF);
    p[6] = (uint8_t)((value >> 48) & 0xFF);
    p[7] = (uint8_t)((value >> 56) & 0xFF);
}

static inline void write_unaligned16(void *up, uint16_t value){
    volatile uint8_t *p = (volatile uint8_t*)up;
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
}

#ifdef __cplusplus
}
#endif