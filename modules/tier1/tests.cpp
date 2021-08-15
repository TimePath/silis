#include "../test/test.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"

using namespace tier1;

using namespace test;

TEST("Array") {
    let array = Array<Int>(3, [](Int i) { return 1 + i; });
    printf("Array: %d %d %d\n", array.get(0).wordValue, array.get(1).wordValue, array.get(2).wordValue);
}
