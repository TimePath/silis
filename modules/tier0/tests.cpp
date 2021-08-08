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
}

USING_STRUCTURED_BINDING

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

TEST("Tuples") {
    var testVar = Tuple{Int(1), Short(2), Byte(3)};
    auto &[_v1, _v2, _v3] = testVar;
    printf("testVar: %s: %d, %d, %d\n", TypeName<decltype(testVar)>(), _v1.wordValue, _v2.wordValue, _v3.wordValue);
}

struct raw {
    cstring value;
};

template<typename T>
struct printer;

template<>
struct printer<raw> {
    static Boolean print(Boolean dry, ref<raw> self, Boolean value) {
        if (value) {
            return false;
        } else {
            if (!dry) printf("%s", self.value);
            return true;
        }
    }
};

template<>
struct printer<cstring> {
    static Boolean print(Boolean dry, ref<cstring> self, Boolean value) {
        if (value) {
            if (!dry) printf("%s", self);
            return true;
        } else {
            if (!dry) printf("?");
            return true;
        }
    }
};

template<>
struct printer<Int> {
    static Boolean print(Boolean dry, ref<Int> self, Boolean value) {
        if (value) {
            if (!dry) printf("%d", Native<Int>(self));
            return true;
        } else {
            if (!dry) printf("?");
            return true;
        }
    }
};

template<Size n, typename... Ts>
struct Statement {
    const SizedArray<cstring, n> strings;
    const Tuple<Ts...> values;

    constexpr void operator()(ref<SourceLocation> loc = SourceLocation::current()) const {
        printf("'");
        printf("/* %s */ ", loc.file);
        printf("%s", strings.data[0]);
        auto printIdentifier = [&]<typename U>(Size i, ref<U> it) -> Size {
            printer<U>::print(false, it, false);
            printf("%s", strings.data[1 + i]);
            return i + 1;
        };
        forEach(values, printIdentifier, Size(0));
        printf("'");
        printf(" -- ");
        constexpr auto printValue = []<typename U>(Size i, ref<U> it) -> Size {
            if (!printer<U>::print(true, it, true)) {
                return i;
            }
            if (i > 0) printf(", ");
            printf("'");
            printer<U>::print(false, it, true);
            printf("'");
            return i + 1;
        };
        forEach(values, printValue, Size(0));
    }
};

template<LiteralString str, typename... Ts>
auto sql(Ts... values) {
    constexpr var N = strtok::count<str>();
    static_assert(sizeof...(Ts) == N - 1, "incorrect number of arguments");
    return Statement<N, Ts...>{strtok::collect<str>(), {values...}};
}

TEST("StrTok") {
    let stmt = sql<"insert into {}(id, name) values ({}, {})">(raw{"table"}, Int(1), "name");
    stmt();
    printf("\n");
}
