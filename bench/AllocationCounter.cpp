#include <cstddef>
#include <cstdlib>
#include <new>

namespace {
    std::size_t allocationCount = 0;
}

std::size_t AllocationCount() {
    return allocationCount;
}

void* operator new(std::size_t size) {
    ++allocationCount;
    if (size == 0) {
        size = 1;
    }
    void* pointer = std::malloc(size);
    if (pointer == nullptr) {
        throw std::bad_alloc();
    }
    return pointer;
}

void operator delete(void* pointer) noexcept {
    std::free(pointer);
}

void operator delete(void* pointer, std::size_t) noexcept {
    std::free(pointer);
}