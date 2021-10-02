#pragma once

#include "../tier2/tier2.hpp"

using namespace tier2;

namespace scriptengine::jvm {
    struct Class;

    struct ClassHandle {
        ptr<Class> handle;

        void release();
    };

    ClassHandle LoadClass(DynArray<Byte> data);
}
