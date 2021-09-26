#pragma once

#include "../tier0/tier0.hpp"

using namespace tier0;

Int interface_open(cstring name);

Int interface_read(Int handle, Span<Byte, 0xffff + 1> span);
