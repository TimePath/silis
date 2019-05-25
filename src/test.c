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
    ret = system(va(__VA_ARGS__)); \
    run_check(ret); \
} while (0)

#ifdef _WIN32
#define run_check(ret) do { \
    printf("run: status: %d\n", ret); \
} while (0)
#else
#define run_check(ret) do { \
    if (WIFEXITED(ret)) printf("run: status: %d\n", WEXITSTATUS(ret)); \
    if (WIFSIGNALED(ret)) printf("run: signal %d\n", WTERMSIG(ret)); \
    ret = WEXITSTATUS(ret) || WTERMSIG(ret); \
} while (0)
#endif

#define run_or_exit(...) do { \
    run(__VA_ARGS__); \
    if (ret) return EXIT_FAILURE; \
} while (0)

int main(int argc, const char *argv[]) {
    (void) argc;
    int ret = 0;
    char const *PWD = getenv("PWD");
    char const *CC = getenv("CC");
    char const *silis = argv[1];
    char const *target = "c";
    char const *dir_in = argv[2];
    char const *dir_out = argv[3];
    char const *test = argv[4];
    printf("PWD=%s\n", PWD);
    printf("CC=%s\n", CC);
    printf("silis=%s\n", silis);
    printf("dir_in=%s\n", dir_in);
    printf("dir_out=%s\n", dir_out);
    printf("test=%s\n", test);
    run_or_exit("%s %s %s %s %s %s.c", silis, target, dir_in, dir_out, test, test);
#ifdef _WIN32
    run_or_exit("\"%s\" %s\\%s.c /Fe:%s\\%s.exe", CC, dir_out, test, dir_out, test);
    run("%s\\%s.exe", dir_out, test);
#else
    run_or_exit("%s %s/%s.c -o %s/%s.exe", CC, dir_out, test, dir_out, test);
    run("%s/%s.exe", dir_out, test);
#endif
    return EXIT_SUCCESS;
}
