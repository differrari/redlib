#include "allocator.hpp"
#include "syscalls/syscalls.h"

void* operator new(size_t size, void* ptr) noexcept {
    return ptr;
}

void* operator new(size_t size) { 
    return (void*)zalloc(size);
}

void* operator new[](size_t size) { 
    return (void*)zalloc(size);
}

//TODO: properly implement these
void operator delete(void* ptr) noexcept {
    free_sized(ptr, 0);
}

void operator delete[](void* ptr) noexcept {
    free_sized(ptr, 0);
}

void operator delete(void* ptr, size_t size) noexcept {
    free_sized(ptr, size);
}

void operator delete[](void* ptr, size_t size) noexcept {
    free_sized(ptr, size);
}
