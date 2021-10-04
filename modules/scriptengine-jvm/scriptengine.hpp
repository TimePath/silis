#pragma once

#include "../tier2/tier2.hpp"

using namespace tier2;

namespace scriptengine::jvm {
    struct Class;

    struct ClassHandle {
        ptr<Class> handle;

        void release();
    };

    struct MethodHandle {
        ClassHandle handle;
        Int index;
    };

    ClassHandle LoadClass(DynArray<Byte> data);

    void LoadCode(MethodHandle handle);
}
