// SPDX-License-Identifier: AFL-3.0
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "../tier0/tier0.hpp"
#include "test.hpp"

using namespace tier0;

extern "C"
LIBTEST_EXPORT
Native<Int> main(Native<Int> argc, Native<ptr<cstring>> argv) {
    setbuf(stdout, nullptr);
    test::main(argc - 1, argv + 1);
    return 0;
}

namespace test {
    static Native<ptr<const Test>> g_head = nullptr;

    CompileTest::CompileTest(cstring_t _file, int_t _id, cstring_t _name, cstring_t _code) noexcept
            : Test{Mode::Compile, g_head, _file, _id, _name}, code_(_code) {
        g_head = this;
    }

    RunTest::RunTest(cstring_t _file, int_t _id, cstring_t _name, runnable_t _run) noexcept
            : Test{Mode::Run, g_head, _file, _id, _name}, run_(_run) {
        g_head = this;
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

    void runTest(Native<ptr<const Test>> test);

    void main(int_t argc, Native<ptr<cstring_t>> argv) {
        if (argc == 0) {
            auto count = 0;
            const auto format = "%s;%c;%s;%d\n";
            for (auto test = g_head; test; test = test->next_) {
                printf(format, test->name_, test->mode_ == Mode::Compile ? 'C' : 'R', test->file_, test->id_);
                ++count;
            }
            if (!count) {
                printf(format, "NOTFOUND", 'R', "", 0);
            }
            return;
        }
        auto target = argv[0];
        auto test = g_head;
        for (; test; test = test->next_) {
            if (target && strcmp(test->name_, target) != 0) {
                continue;
            }
            runTest(test);
        }
    }

    void runTest(Native<ptr<const Test>> test) {
        switch (test->mode_) {
            case Mode::Compile: {
                auto compileTest = static_cast<Native<ptr<const CompileTest>>>(test);
                printf(".compile %s\n", compileTest->code_);
                break;
            }
            case Mode::Run: {
                auto runTest = static_cast<Native<ptr<const RunTest>>>(test);
                runTest->run_();
                break;
            }
        }
    }
}
