#pragma once

#include "../tier1/tier1.hpp"

namespace kernel {
    using namespace tier1;
}

namespace kernel {
    DynArray<Byte> file_read(cstring path);

    Int interface_open(cstring name);

    Int interface_read(Int handle, Span<Byte, Size(0xffff + 1)> span);
}
