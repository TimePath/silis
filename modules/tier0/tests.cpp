#include "../test/test.hpp"

#include "../tier0/tier0.hpp"

using namespace tier0;

using namespace test;

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
        printf("/* %s */ ", loc._file);
        printf("%s", strings._data[0]);
        auto printIdentifier = [&]<typename U>(Size i, ref<U> it) -> Size {
            printer<U>::print(false, it, false);
            printf("%s", strings._data[1 + i]);
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

template<typename T>
struct Traced {
    Boolean live = true;
    T _value;

    ~Traced() {
        if (!live) return;
        printf("%s: dtor()\n", TypeName<Traced>());
    }

    explicit Traced(T value) : _value(move(value)) {
        printf("%s: ctor(%s)\n", TypeName<Traced>(), TypeName<T>());
    }

    explicit Traced(movable<Traced> other) : _value(move(other._value)) {
        other.live = false;
        printf("%s: ctor(%s &&)\n", TypeName<Traced>(), TypeName<T>());
    }

    explicit Traced(ref<Traced> other) : _value(other._value) {
        printf("%s: ctor(%s const &)\n", TypeName<Traced>(), TypeName<T>());
    }
};

template<typename T>
Traced(T value) -> Traced<T>;

TEST("Optional") {
    {
        var o1 = Optional<Traced<Int>>::of(Traced(Int(1)));
        if (!o1.hasValue()) {
            printf("O1: Empty\n");
            return;
        }
        printf("O1: Value: %d\n", Native<Int>(o1.value()._value));
    }
    {
        var o2 = Optional<Traced<Int>>::empty();
        if (!o2.hasValue()) {
            printf("O2: Empty\n");
            return;
        }
        printf("O2: Value: %d\n", Native<Int>(o2.value()._value));
    }
}

enum struct ErrorCode {
    Ok,
    BadNumber,
};

TEST("Result") {
    {
        var r1 = Result<Traced<Int>, Traced<ErrorCode>>::value(Traced(Int(1)));
        if (r1.isError()) {
            printf("R1: Error: %d\n", Native<Int>(r1.error()._value));
            return;
        }
        printf("R1: Value: %d\n", Native<Int>(r1.value()._value));
    }
    {
        var r2 = Result<Traced<Int>, Traced<ErrorCode>>::error(Traced(ErrorCode::BadNumber));
        if (r2.isError()) {
            printf("R2: Error: %d\n", Native<Int>(r2.error()._value));
            return;
        }
        printf("R2: Value: %d\n", Native<Int>(r2.value()._value));
    }
}

TEST("Variant") {
    using V = Variant<Traced<Short>, Traced<Int>, Traced<Long>>;
    {
        var v1 = V::of<1>(Traced(Int(1)));
        printf("V1: Value: %d\n", Native<Int>(v1.template get<1>()._value));
    }
    {
        var v2 = V::of<0>(Traced(Short(1)));
        v2.set<1>(Traced(Int(2)));
        printf("V2: Value: %d\n", Native<Int>(v2.template get<1>()._value));
    }
}

TEST("Range") {
    for (var it : Range<Int>::until(Int(0), Int(5))) {
        printf("%d", Native<Int>(it));
    }
    for (var it : Range<Int>::until(Int(5), Int(10))) {
        printf("%d", Native<Int>(it));
    }
    printf("\n");
}
