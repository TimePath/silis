#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "test.hpp"

int main(int argc, char const **argv) {
    setbuf(stdout, nullptr);
    test::main(argc - 1, argv + 1);
    return 0;
}

namespace test {
    Test const *head = nullptr;

    Test::Test(cstring_t name, runnable_t run) noexcept: next(head), name(name), run(run) {
        head = this;
    }

    inline void main(int_t argc, cstring_t *argv) {
        if (argc == 0) {
            auto count = 0;
            for (auto test = head; test; test = test->next) {
                printf("%s\n", test->name);
                ++count;
            }
            if (!count) {
                printf("%s\n", "NOTFOUND");
            }
            return;
        }
        auto target = argv[0];
        auto test = head;
        for (; test && strcmp(test->name, target) != 0; test = test->next);
        if (test) {
            test->run();
        }
    }

    int_t strcmp(cstring_t a, cstring_t b) {
        auto i = 0;
        for (; a[i] == b[i] && a[i] && b[i]; ++i);
        return a[i] - b[i];
    }

    void printf(const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
    }

    void abort() { ::abort(); }
}
