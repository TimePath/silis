#include "../test/test.hpp"

#include "../tier0/tier0.hpp"

TEST("False") {
    let value = Boolean(false);
    printf("false: %d\n", value.wordValue);
}

TEST("True") {
    let value = Boolean(true);
    printf("true: %d\n", value.wordValue);
}

TEST("TypeName") {
    let value = TypeName<Boolean>();
    printf("TypeName<Boolean>: %s\n", value);
    if (!strequal(value, "tier0::detail::Word<bool>")) abort();
}

TEST("Binding") {
    struct Test {
        Int i;
        Short j;
        Int k;

        using elements = tuple_elements_builder<>
        ::add<&Test::i>
        ::add<&Test::j>
        ::add<&Test::k>
        ::build;
    };

    var testVar = Test{
            .i = 1,
            .j = Short(2),
            .k = 3,
    };
    auto &[_v1, _v2, _v3] = testVar;
    _v1 = _v1 + 1;
    printf("testVar: %d, %d, %d\n", _v1.wordValue, _v2.wordValue, _v3.wordValue);

    let testLet = Test{
            .i = 1,
            .j = Short(2),
            .k = 3,
    };
    auto &[_l1, _l2, _l3] = testLet;
    printf("testLet: %d, %d, %d\n", _l1.wordValue, _l2.wordValue, _l3.wordValue);
}
