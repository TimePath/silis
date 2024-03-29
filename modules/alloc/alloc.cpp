// SPDX-License-Identifier: AFL-3.0
#define DEBUG 0

#if DEBUG

#include <cstdio>
#include <cstdlib>
#include <cstring>

#endif

#include "alloc.hpp"

using namespace tier0;

#if !DEBUG

Native<ptr<void>> operator_new(Native<Size> count, AllocInfo) {
    return ::operator_new(count);
}

Native<ptr<void>> operator_new[](Native<Size> count, AllocInfo) {
    return ::operator_new[](count);
}

#else

namespace {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
    let DEBUG_PRINT = Boolean([]() {
        let p = getenv("DEBUG_ALLOC");
        if (!p) {
            return false;
        }
        return p[0] != '0';
    }());
    let RUNNING_ON_VALGRIND = Boolean([]() {
        let p = getenv("LD_PRELOAD");
        if (!p) {
            return false;
        }
        return strstr(p, "/valgrind/") != nullptr || strstr(p, "/vgpreload") != nullptr;
    }());
#pragma clang diagnostic pop

    struct MemoryBlock {
    private:
        inline static Size ids_ = Size(0);
    public:
        Size id_;
        IntrusiveLinks<MemoryBlock> links_;
        AllocInfo info_;

        explicit MemoryBlock(AllocInfo info) : id_(ids_ = ids_ + 1), links_(), info_(move(info)) {}
    };

    IntrusiveList<MemoryBlock, &MemoryBlock::links_> g_blocks;

    ptr<Byte> alloc_ctor(ptr<MemoryBlock> allocation, Size size, AllocInfo info) {
        var header = allocation;
        let payload = ptr<Byte>::reinterpret(allocation + 1);
        info.size_ = size;
        var &block = *new(header) MemoryBlock(info);
        if (DEBUG_PRINT) {
            fprintf(stderr, "alloc_ctor %p = %" PRIuZ " %s\n", Native<ptr<void>>(&block), Native<Size>(info.size_),
                    info.name_);
        }
        g_blocks.add(block);
        return payload;
    }

    ptr<Byte> alloc_dtor(ptr<MemoryBlock> payload) {
        let header = payload - 1;
        let allocation = ptr<Byte>::reinterpret(header);
        var &block = *header;
        if (DEBUG_PRINT) {
            let info = block.info_;
            fprintf(stderr, "alloc_dtor %p = %" PRIuZ " %s\n", Native<ptr<void>>(&block), Native<Size>(info.size_),
                    info.name_);
        }
        g_blocks.remove(block);
        block.~MemoryBlock();
        return allocation;
    }
}

// https://en.cppreference.com/w/cpp/memory/new/operator_new

Native<ptr<void>> operator_new(Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_new(count);
    }
    let memory = ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

Native<ptr<void>> operator_new[](Native<Size> count, AllocInfo info) {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_new[](count);
    }
    let memory = ptr<MemoryBlock>(::malloc(sizeof(MemoryBlock) + count));
    return alloc_ctor(memory, count, info);
}

// https://en.cppreference.com/w/cpp/memory/new/operator_delete

void operator_delete(Native<ptr<void>> obj) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_delete(obj);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator_delete[](Native<ptr<void>> obj) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_delete[](obj);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator_delete(Native<ptr<void>> obj, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_delete(obj, sz);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

void operator_delete[](Native<ptr<void>> obj, Native<Size> sz) noexcept {
    if (RUNNING_ON_VALGRIND) {
        return ::operator_delete[](obj, sz);
    }
    let memory = ptr<MemoryBlock>(obj);
    ::free(alloc_dtor(memory));
}

#endif
