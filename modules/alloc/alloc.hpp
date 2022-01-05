#pragma once
// SPDX-License-Identifier: AFL-3.0

#ifdef LIBALLOC_EXPORTS
#define LIBALLOC_EXPORT EXPORT_DLLEXPORT
#else
#define LIBALLOC_EXPORT EXPORT_DLLIMPORT
#endif

#include "../tier0/tier0.hpp"

// memory
namespace tier0 {
    struct AllocInfo {
        tier0::cstring name_;
        tier0::Size elementSize_;
        tier0::Size size_;

        template<typename T>
        static AllocInfo of() { return {tier0::TypeName<T>(), sizeof(T), tier0::Size(0)}; }
    };
}

LIBALLOC_EXPORT
tier0::Native<tier0::ptr<void>> operator_new(tier0::Native<tier0::Size> count, tier0::AllocInfo info);

LIBALLOC_EXPORT
tier0::Native<tier0::ptr<void>> operator_new[](tier0::Native<tier0::Size> count, tier0::AllocInfo info);

#if COMPILER_IS_MSVC
#define LIBALLOC_MAYBE_EXPORT
#else
#define LIBALLOC_MAYBE_EXPORT LIBALLOC_EXPORT
#endif

LIBALLOC_MAYBE_EXPORT
void operator_delete(tier0::Native<tier0::ptr<void>> obj) noexcept;

LIBALLOC_MAYBE_EXPORT
void operator_delete[](tier0::Native<tier0::ptr<void>> obj) noexcept;

LIBALLOC_MAYBE_EXPORT
void operator_delete(tier0::Native<tier0::ptr<void>> obj, tier0::Native<tier0::Size> sz) noexcept;

LIBALLOC_MAYBE_EXPORT
void operator_delete[](tier0::Native<tier0::ptr<void>> obj, tier0::Native<tier0::Size> sz) noexcept;
