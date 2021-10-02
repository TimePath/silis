#pragma once

#include "../tier2/tier2.hpp"

using namespace tier2;

DynArray<Byte> file_read(cstring name);

Int interface_open(cstring name);

Int interface_read(Int handle, Span<Byte, Size(0xffff + 1)> span);
