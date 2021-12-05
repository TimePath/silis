#pragma once

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

tier0::Native<tier0::ptr<void>> operator_new(tier0::Native<tier0::Size> count, tier0::AllocInfo info);

tier0::Native<tier0::ptr<void>> operator_new[](tier0::Native<tier0::Size> count, tier0::AllocInfo info);
