#pragma once

namespace test {
    namespace {
        using int_t = decltype(0);
        using cstring_t = decltype(&""[0]);
        using runnable_t = void (*)();
    }

    struct Test {
        Test const *next;
        cstring_t _name;
        runnable_t _run;

        Test(cstring_t name, runnable_t run) noexcept;
    };

    void main(int_t argc, cstring_t *argv);

    int_t strcmp(cstring_t a, cstring_t b);

    inline int_t strequal(cstring_t a, cstring_t b) { return strcmp(a, b) == 0; }

    [[gnu::format(printf, 1, 2)]]
    void printf(cstring_t format, ...);

    [[noreturn]]
    void abort();
}

#define TEST(name) TEST_1(name, __COUNTER__)
#define TEST_1(name, id) TEST_2(name, id)
#define TEST_2(name, id) \
    static void test##id(); \
    static Test test##id##_global(name, test##id); \
    void test##id()
