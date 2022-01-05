// SPDX-License-Identifier: AFL-3.0
#include "../test/test.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

using namespace tier1;

using namespace test;

TEST("DynArray") {
    let array = DynArray<Int>(3, [](Int i) { return 1 + i; });
    printf("DynArray: %d %d %d\n", Native<Int>(array.get(0)), Native<Int>(array.get(1)), Native<Int>(array.get(2)));
}
