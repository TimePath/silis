#include "../test/test.hpp"

#include "../tier0/tier0.hpp"
#include "../tier1/tier1.hpp"
#include "../tier2/tier2.hpp"

using namespace tier2;

using namespace test;

TEST("List") {
    var listVar = List<String>();
    listVar.add("One");
    listVar.add("Two");
    listVar.add("Three");
    printf("List:");
    let listLet = listVar;
    for (auto &it : listLet) {
        printf(" %s", cstring(it));
    }
    printf("\n");
}
