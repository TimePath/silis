#include "../test/test.hpp"

#include "../tier0/tier0.hpp"

using namespace tier0;

using namespace test;

TEST("False") {
    let value = Boolean(false);
    printf("false: %d\n", Native<Boolean>(value));
}

TEST("True") {
    let value = Boolean(true);
    printf("true: %d\n", Native<Boolean>(value));
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
    printf("testVar: %d, %d, %d\n", Native<Int>(_v1), Native<Short>(_v2), Native<Int>(_v3));

    let testLet = Test{
            .i = 1,
            .j = Short(2),
            .k = 3,
    };
    auto &[_l1, _l2, _l3] = testLet;
    printf("testLet: %d, %d, %d\n", Native<Int>(_l1), Native<Short>(_l2), Native<Int>(_l3));
}

TEST("Tuples") {
    var testVar = Tuple{Int(1), Short(2), Byte(3)};
    auto &[_v1, _v2, _v3] = testVar;
    printf("testVar: %s: %d, %d, %d\n", TypeName<decltype(testVar)>(), Native<Int>(_v1), Native<Short>(_v2),
           Native<Byte>(_v3));
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
    const Array<cstring, n> strings;
    const Tuple<Ts...> values;

    constexpr void operator()(ref<SourceLocation> loc = SourceLocation::current()) const {
        printf("'");
        printf("/* %s */ ", loc.file_);
        printf("%s", strings.data_[0]);
        auto printIdentifier = [&]<typename U>(Size i, ref<U> it) -> Size {
            printer<U>::print(false, it, false);
            printf("%s", strings.data_[1 + i]);
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

TEST_COMPILE("StrTok argcount", {
    let stmt = sql<"Hello {}">();
})

template<typename T>
struct Traced {
    Boolean live_;
    T value_;

    ~Traced() {
        if (!live_) return;
        printf("%s: dtor()\n", TypeName<Traced>());
    }

    explicit Traced(T value) : live_(true), value_(move(value)) {
        printf("%s: ctor(%s)\n", TypeName<Traced>(), TypeName<T>());
    }

    explicit Traced(movable<Traced> other) : live_(exchange(other.live_, false)), value_(move(other.value_)) {
        printf("%s: ctor(%s &&)\n", TypeName<Traced>(), TypeName<T>());
    }

    explicit Traced(ref<Traced> other) : live_(other.live_), value_(other.value_) {
        printf("%s: ctor(%s const &)\n", TypeName<Traced>(), TypeName<T>());
    }
};

template<typename T>
Traced(T value) -> Traced<T>;

TEST("Pointer") {
    {
        var set = Boolean(false);
        try {
            var p1 = Traced<ptr<Int>>(nullptr);
            set = true;
        } catch (...) {
        }
        printf("p1: %d\n", Native<Boolean>(set));
    }
    {
        var i = Int(0);
        var p1 = Traced<ptr<Int>>(&i);
        printf("p2: %d\n", !!p1.value_);
    }
}

TEST("Optional") {
    {
        var o1 = Optional<Traced<Int>>::of(Traced(Int(1)));
        if (!o1.hasValue()) {
            printf("O1: Empty\n");
            return;
        }
        printf("O1: Value: %d\n", Native<Int>(o1.value().value_));
    }
    {
        var o2 = Optional<Traced<Int>>::empty();
        if (!o2.hasValue()) {
            printf("O2: Empty\n");
            return;
        }
        printf("O2: Value: %d\n", Native<Int>(o2.value().value_));
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
            printf("R1: Error: %d\n", Native<Int>(r1.error().value_));
            return;
        }
        printf("R1: Value: %d\n", Native<Int>(r1.value().value_));
    }
    {
        var r2 = Result<Traced<Int>, Traced<ErrorCode>>::error(Traced(ErrorCode::BadNumber));
        if (r2.isError()) {
            printf("R2: Error: %d\n", Native<Int>(r2.error().value_));
            return;
        }
        printf("R2: Value: %d\n", Native<Int>(r2.value().value_));
    }
}

TEST("Variant") {
    using V = Variant<Byte, Traced<Short>, Traced<Int>, Traced<Long>>;
    {
        var v1 = V::of<Byte(2)>(Traced(Int(1)));
        printf("V1: Value: %d\n", Native<Int>(v1.get<Byte(2)>().value_));
    }
    {
        var v2 = V::of<Byte(1)>(Traced(Short(1)));
        v2.set<Byte(2)>(Traced(Int(2)));
        printf("V2: Value: %d\n", Native<Int>(v2.get<Byte(2)>().value_));
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
