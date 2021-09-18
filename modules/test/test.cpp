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
    static Test const *head = nullptr;

    CompileTest::CompileTest(cstring_t _file, int_t _id, cstring_t _name, cstring_t _code) noexcept
            : Test{Mode::Compile, head, _file, _id, _name}, code(_code) {
        head = this;
    }

    RunTest::RunTest(cstring_t _file, int_t _id, cstring_t _name, runnable_t _run) noexcept
            : Test{Mode::Run, head, _file, _id, _name}, run(_run) {
        head = this;
    }

    [[gnu::format(printf, 1, 2)]]
    void printf(cstring_t format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
    }

    [[noreturn]]
    void abort() { ::abort(); }

    static int_t strcmp(cstring_t a, cstring_t b) {
        auto i = 0;
        for (; a[i] == b[i] && a[i] && b[i]; ++i);
        return a[i] - b[i];
    }

    inline void main(int_t argc, cstring_t *argv) {
        if (argc == 0) {
            auto count = 0;
            const auto format = "%s;%c;%s;%d\n";
            for (auto test = head; test; test = test->next) {
                printf(format, test->name, test->mode == Mode::Compile ? 'C' : 'R', test->file, test->id);
                ++count;
            }
            if (!count) {
                printf(format, "NOTFOUND", 'R', "", 0);
            }
            return;
        }
        auto target = argv[0];
        auto test = head;
        for (; test && strcmp(test->name, target) != 0; test = test->next);
        if (!test) return;
        switch (test->mode) {
            case Mode::Compile: {
                auto compileTest = static_cast<CompileTest const *>(test);
                printf(".compile %s\n", compileTest->code);
                break;
            }
            case Mode::Run: {
                auto runTest = static_cast<RunTest const *>(test);
                runTest->run();
                break;
            }
        }
    }
}
