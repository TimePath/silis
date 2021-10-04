#pragma once

#include "../tier2/tier2.hpp"

using namespace tier2;

namespace scriptengine::jvm {
    struct Class;

    struct ClassHandle {
        ptr<Class> handle_;

        void release();
    };

    struct MethodHandle {
        ClassHandle handle_;
        Int index_;
    };

    ClassHandle LoadClass(DynArray<Byte> data);

    void LoadCode(MethodHandle handle);
}
