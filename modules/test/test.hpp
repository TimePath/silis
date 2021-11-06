#pragma once

#include "../tier0/tier0.hpp"

namespace test {
    using namespace tier0;
}

namespace test {
    namespace {
        using int_t = Native<Int>;
        using cstring_t = cstring;
        using runnable_t = Native<ptr<void()>>;
    }

    enum class Mode {
        Compile,
        Run,
    };

    struct Test {
        Mode mode_;
        Native<ptr<const Test>> next_;
        cstring_t file_;
        int_t id_;
        cstring_t name_;
    };

    struct CompileTest : Test {
        cstring_t code_;

        CompileTest(cstring_t file, int_t id, cstring_t name, cstring_t code) noexcept;
    };

    struct RunTest : Test {
        runnable_t run_;

        RunTest(cstring_t file, int_t id, cstring_t name, runnable_t run) noexcept;
    };

    [[gnu::format(printf, 1, 2)]]
    void printf(cstring_t format, ...);

    [[noreturn]]
    void abort();

    void main(int_t argc, Native<ptr<cstring_t>> argv);
}

#define TEST_COMPILE(name, code) TEST_COMPILE_1(name, __COUNTER__, code)
#define TEST_COMPILE_1(name, id, code) TEST_COMPILE_2(name, id, code)
#define TEST_COMPILE_2(name, id, code) \
    [[maybe_unused]] static void test##id(); \
    static CompileTest test##id##_global(__FILE__, id, name, #code); \
    void test##id() TEST_COMPILE_COND(id, code)

#if !defined(__TEST_COMPILE)
#define TEST_COMPILE_COND(id, code) TEST_COMPILE_COND_RET_0(code)
#else
#define TEST_COMPILE_COND(id, code) TEST_COMPILE_COND_1(__TEST_##id, code)
#define TEST_COMPILE_COND_1(id, code) TEST_COMPILE_COND_2(id, code)
#define TEST_COMPILE_COND_2(id, code) TEST_COMPILE_COND_RET_##id(code)
#endif
#define TEST_COMPILE_COND_RET_0(code) {}
#define TEST_COMPILE_COND_RET_1(code) code

#define TEST(name) TEST_1(name, __COUNTER__)
#define TEST_1(name, id) TEST_2(name, id)
#define TEST_2(name, id) \
    static void test##id(); \
    static RunTest test##id##_global(__FILE__, id, name, test##id); \
    void test##id()
