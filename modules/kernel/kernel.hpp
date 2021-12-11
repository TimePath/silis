#pragma once

#ifdef LIBKERNEL_EXPORTS
#define LIBKERNEL_EXPORT EXPORT_DLLEXPORT
#else
#define LIBKERNEL_EXPORT EXPORT_DLLIMPORT
#endif

#include "../tier1/tier1.hpp"

namespace kernel {
    LIBKERNEL_EXPORT void dummy();

    using namespace tier1;
}

namespace kernel {
    LIBKERNEL_EXPORT
    DynArray<Byte> file_read(cstring path);

    LIBKERNEL_EXPORT
    Int interface_open(cstring name);

    LIBKERNEL_EXPORT
    Int interface_read(Int handle, Span<Byte, Size(0xffff + 1)> span);
}
