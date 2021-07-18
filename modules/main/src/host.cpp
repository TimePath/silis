#include <cstdlib>
#include <cstring>

#include "../../tier2/tier2.hpp"

#define IMPLEMENTS_NEW 1

namespace {
    class MemoryBlock {
    private:
        inline static ULong ids = ULong(0);
    public:
        ULong id;
        IntrusiveLinks<MemoryBlock> links;
        AllocInfo info;

        ~MemoryBlock() = default;

        explicit MemoryBlock(AllocInfo info) : id(ids = ids + 1), links(), info(move(info)) {}
    };

    IntrusiveList<MemoryBlock, &MemoryBlock::links> blocks;

    ptr<Native<Byte>> alloc_ctor(ptr<Native<Byte>> memory, ULong size, AllocInfo info) {
        let ptr = memory + sizeof(MemoryBlock);
        info.size = size;
        var &block = *new(memory) MemoryBlock(info);
        blocks.add(&block);
        return ptr;
    }

    ptr<Native<Byte>> alloc_dtor(ptr<Native<Byte>> ptr) {
        let memory = ptr - sizeof(MemoryBlock);
        var &block = *(::ptr<MemoryBlock>) memory;
        blocks.remove(&block);
        block.~MemoryBlock();
        return memory;
    }

    let RUNNING_ON_VALGRIND = Boolean([]() noexcept {
        let p = getenv("LD_PRELOAD");
        if (!p) {
            return false;
        }
        return strstr(p, "/valgrind/") != nullptr || strstr(p, "/vgpreload") != nullptr;
    }());
}

// https://en.cppreference.com/w/cpp/memory/new/operator_new

#if IMPLEMENTS_NEW
ptr<void> operator new(size_t count, ptr<void> ptr) noexcept { return ptr; }

ptr<void> operator new[](size_t count, ptr<void> ptr) noexcept { return ptr; }
#endif

ptr<void> operator new(size_t count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new(count);
    }
    let memory = (ptr<Native<tier0::Byte>>) ::malloc(sizeof(MemoryBlock) + count);
    return alloc_ctor(memory, count, info);
}

ptr<void> operator new[](size_t count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new[](count);
    }
    let memory = (ptr<Native<tier0::Byte>>) ::malloc(sizeof(MemoryBlock) + count);
    return alloc_ctor(memory, count, info);
}

// https://en.cppreference.com/w/cpp/memory/new/operator_delete

void operator delete(ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr);
    }
    let memory = (::ptr<Native<tier0::Byte>>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete[](ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr);
    }
    let memory = (::ptr<Native<tier0::Byte>>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete(ptr<void> ptr, size_t sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr, sz);
    }
    let memory = (::ptr<Native<tier0::Byte>>) ptr;
    ::free(alloc_dtor(memory));
}

void operator delete[](ptr<void> ptr, size_t sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr, sz);
    }
    let memory = (::ptr<Native<tier0::Byte>>) ptr;
    ::free(alloc_dtor(memory));
}

#if IMPLEMENTS_NEW
void operator delete(ptr<void> ptr, ::ptr<void> place) noexcept {}

void operator delete[](ptr<void> ptr, ::ptr<void> place) noexcept {}
#endif
