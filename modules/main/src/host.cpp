#include <cstdlib>
#include <cstring>

#include "../../tier2/tier2.hpp"

#define IMPLEMENTS_NEW 1

namespace {
    struct MemoryBlock {
    private:
        inline static Size ids = Size(0);
    public:
        Size id;
        IntrusiveLinks<MemoryBlock> links;
        AllocInfo info;

        ~MemoryBlock() = default;

        explicit MemoryBlock(AllocInfo info) : id(ids = ids + 1), links(), info(move(info)) {}
    };

    IntrusiveList<MemoryBlock, &MemoryBlock::links> blocks;

    mut_ptr<Byte> alloc_ctor(mut_ptr<Byte> memory, Size size, AllocInfo info) {
        let ptr = memory + sizeof(MemoryBlock);
        info.size = size;
        var &block = *new(memory) MemoryBlock(info);
        blocks.add(block);
        return ptr;
    }

    mut_ptr<Byte> alloc_dtor(mut_ptr<Byte> ptr) {
        let memory = ptr - sizeof(MemoryBlock);
        var &block = *(mut_ptr<MemoryBlock>) memory;
        blocks.remove(block);
        block.~MemoryBlock();
        return memory;
    }

    let RUNNING_ON_VALGRIND = Boolean([]() {
        let p = getenv("LD_PRELOAD");
        if (!p) {
            return false;
        }
        return strstr(p, "/valgrind/") != nullptr || strstr(p, "/vgpreload") != nullptr;
    }());
}

// https://en.cppreference.com/w/cpp/memory/new/operator_new

#if IMPLEMENTS_NEW

mut_ptr<void> operator new(Native<Size> count, mut_ptr<void> ptr) noexcept {
    (void) count;
    return ptr;
}

mut_ptr<void> operator new[](Native<Size> count, mut_ptr<void> ptr) noexcept {
    (void) count;
    return ptr;
}

#endif

mut_ptr<void> operator new(Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new(count);
    }
    let memory = (mut_ptr<Byte>) ::malloc(sizeof(MemoryBlock) + count);
    return alloc_ctor(memory, count, info);
}

mut_ptr<void> operator new[](Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new[](count);
    }
    let memory = (mut_ptr<Byte>) ::malloc(sizeof(MemoryBlock) + count);
    return alloc_ctor(memory, count, info);
}

// https://en.cppreference.com/w/cpp/memory/new/operator_delete

void operator delete(mut_ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr);
    }
    let memory = (mut_ptr<Byte>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete[](mut_ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr);
    }
    let memory = (mut_ptr<Byte>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete(mut_ptr<void> ptr, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr, sz);
    }
    let memory = (mut_ptr<Byte>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete[](mut_ptr<void> ptr, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr, sz);
    }
    let memory = (mut_ptr<Byte>) ptr;
    ::free(alloc_dtor(memory));
}

#if IMPLEMENTS_NEW

void operator delete(mut_ptr<void> ptr, mut_ptr<void> place) noexcept {
    (void) ptr;
    (void) place;
}

void operator delete[](mut_ptr<void> ptr, mut_ptr<void> place) noexcept {
    (void) ptr;
    (void) place;
}

#endif
