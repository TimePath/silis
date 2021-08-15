#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../tier2/tier2.hpp"

using namespace tier2;

namespace {
    struct MemoryBlock {
    private:
        inline static Size ids = Size(0);
    public:
        Size id;
        IntrusiveLinks<MemoryBlock> links;
        AllocInfo _info;

        ~MemoryBlock() = default;

        explicit MemoryBlock(AllocInfo info) : id(ids = ids + 1), links(), _info(move(info)) {}
    };

    IntrusiveList<MemoryBlock, &MemoryBlock::links> blocks;

    let DEBUG = Boolean([]() {
        let p = getenv("DEBUG_ALLOC");
        if (!p) {
            return false;
        }
        return p[0] != '0';
    }());

    mut_ptr<Byte> alloc_ctor(mut_ptr<MemoryBlock> allocation, Size size, AllocInfo info) {
        let header = allocation;
        let payload = mut_ptr<Byte>(allocation + 1);
        info.size = size;
        if (DEBUG) {
            fprintf(stderr, "alloc %lu %s\n", info.size.wordValue, info.name);
        }
        var &block = *new(header) MemoryBlock(info);
        blocks.add(block);
        return payload;
    }

    mut_ptr<Byte> alloc_dtor(mut_ptr<MemoryBlock> payload) {
        let header = payload - 1;
        let allocation = mut_ptr<Byte>(header);
        var &block = *header;
        blocks.remove(block);
        block.~MemoryBlock();
        return allocation;
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

mut_ptr<void> operator new(Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new(count);
    }
    let memory = mut_ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

mut_ptr<void> operator new[](Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new[](count);
    }
    let memory = mut_ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

// https://en.cppreference.com/w/cpp/memory/new/operator_delete

void operator delete(mut_ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr);
    }
    let memory = mut_ptr<MemoryBlock>(ptr);
    ::free(alloc_dtor(memory));
}

void operator delete[](mut_ptr<void> ptr) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr);
    }
    let memory = mut_ptr<MemoryBlock>(ptr);
    ::free(alloc_dtor(memory));
}

void operator delete(mut_ptr<void> ptr, Native<Size> sz) noexcept;

void operator delete(mut_ptr<void> ptr, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(ptr, sz);
    }
    let memory = mut_ptr<MemoryBlock>(ptr);
    ::free(alloc_dtor(memory));
}

void operator delete[](mut_ptr<void> ptr, Native<Size> sz) noexcept;

void operator delete[](mut_ptr<void> ptr, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](ptr, sz);
    }
    let memory = mut_ptr<MemoryBlock>(ptr);
    ::free(alloc_dtor(memory));
}
