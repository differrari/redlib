#include "allocator.hpp"
#include "allocate.h"

void* operator new(size_t size, void* ptr) noexcept {
    return ptr;
}

void* operator new(size_t size) { 
    return (void*)zalloc(size);
}

void* operator new[](size_t size) { 
    return (void*)zalloc(size);
}

void operator delete(void* ptr) noexcept {
    release(ptr);
}

void operator delete[](void* ptr) noexcept {
    release(ptr);
}

void operator delete(void* ptr, size_t size) noexcept {
    release(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    release(ptr);
}
