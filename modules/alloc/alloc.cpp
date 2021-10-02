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

    ptr<Byte> alloc_ctor(ptr<MemoryBlock> allocation, Size size, AllocInfo info) {
        var header = allocation;
        let payload = ptr<Byte>::reinterpret(allocation + 1);
        info.size = size;
        var &block = *new(header) MemoryBlock(info);
        if (DEBUG) {
            fprintf(stderr, "alloc_ctor %p = %lu %s\n", Native<ptr<void>>(&block), info.size.wordValue, info.name);
        }
        blocks.add(block);
        return payload;
    }

    ptr<Byte> alloc_dtor(ptr<MemoryBlock> payload) {
        let header = payload - 1;
        let allocation = ptr<Byte>::reinterpret(header);
        var &block = *header;
        if (DEBUG) {
            let info = block._info;
            fprintf(stderr, "alloc_dtor %p = %lu %s\n", Native<ptr<void>>(&block), info.size.wordValue, info.name);
        }
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

Native<ptr<void>> operator new(Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new(count);
    }
    let memory = ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

Native<ptr<void>> operator new[](Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator new[](count);
    }
    let memory = ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

// https://en.cppreference.com/w/cpp/memory/new/operator_delete

void operator delete(Native<ptr<void>> obj) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(obj);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator delete[](Native<ptr<void>> obj) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](obj);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator delete(Native<ptr<void>> obj, Native<Size> sz) noexcept;

void operator delete(Native<ptr<void>> obj, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete(obj, sz);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator delete[](Native<ptr<void>> obj, Native<Size> sz) noexcept;

void operator delete[](Native<ptr<void>> obj, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator delete[](obj, sz);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}
