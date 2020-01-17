#pragma once

#if !defined(static_assert)
#define static_assert(expr, message) extern char (*__static_assert(void)) [sizeof(char[1 - 2*!(expr)])]
#endif
