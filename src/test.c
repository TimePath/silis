#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>

static char _va_buf[1024];

#define va(...) (sprintf(_va_buf, __VA_ARGS__), _va_buf)

#define run(...) do { \
    printf("run: "); \
    printf(__VA_ARGS__); \
    printf("\n"); \
    ret = WEXITSTATUS(system(va(__VA_ARGS__))); \
} while (0)

#define run_or_exit(...) do { \
    run(__VA_ARGS__); \
    if (ret) goto exit; \
} while (0)

int main(int argc, const char *argv[]) {
    (void) argc;
    int ret = 0;
    char const *target = "c";
    char const *PWD = getenv("PWD");
    char const *CC = getenv("CC");
    char const *silis = argv[1];
    char const *test = argv[2];
    printf("PWD=%s\n", PWD);
    printf("CC=%s\n", CC);
    printf("silis=%s\n", silis);
    printf("test=%s\n", test);
    run_or_exit("%s %s %s %s.c", silis, target, test, test);
#ifdef _WIN32
    run_or_exit("\"%s\" %s.c /Fe:%s.exe", CC, test, test);
#else
    run_or_exit("%s %s.c -o %s.exe", CC, test, test);
#endif
    run("%s.exe", test);
    exit:
    printf("Exit code: %d\n", ret);
    return ret;
}
