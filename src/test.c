#include <_prelude/probe.h>

#include <lib/macro.h>

#define _CRT_SECURE_NO_WARNINGS
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#if TARGET_OS_WIN
#pragma warning(push, 0)
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if TARGET_OS_WIN

#include <Windows.h>

#pragma warning(pop)

#else

#include <spawn.h>
#include <sys/wait.h>

#endif

#define va_use(args, a, ...) do { \
    va_list args; \
    va_start(args, a); \
    __VA_ARGS__ \
    va_end(args); \
} while (0)

static char const *va(char const *fmt, ...) {
    static char va_buf[1024];
    static char *va_ptr = va_buf;
    char const *ret = va_ptr;
    DIAG_PUSH
    DIAG_IGNORE("-Wformat-nonliteral")
    va_use(args, fmt, {
        va_ptr += vsprintf(va_ptr, fmt, args) + 1;
    });
    DIAG_POP
    return ret;
}

#define run_or_exit(...) do { \
    run(__VA_ARGS__); \
    if (ret) return EXIT_FAILURE; \
} while (0)

#define run(...) do { \
    char const *_argv[] = { __VA_ARGS__, NULL }; \
    printf("run:"); \
    for (char const **arg = _argv; *arg; ++arg) printf(" %s", *arg); \
    printf("\n"); \
    fflush(NULL); \
    ret = spawn(_argv); \
    run_check(ret); \
} while (0)

#if TARGET_OS_WIN
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

#if TARGET_OS_WIN

typedef PROCESS_INFORMATION pid_t;
typedef void posix_spawn_file_actions_t;
typedef void posix_spawnattr_t;

int posix_spawn(
        pid_t *pid,
        const char *path,
        const posix_spawn_file_actions_t *file_actions,
        const posix_spawnattr_t *attrp,
        const char *argv[],
        const char *envp[]
);

pid_t waitpid(pid_t pid, int *status, int options);

#endif

static char *getcwd_full(void);

static int spawn(char const *argv[]) {
    pid_t pid;
    int ret = posix_spawn(&pid, argv[0], NULL, NULL, (void *) argv, NULL);
    if (ret) return ret;
    waitpid(pid, &ret, 0);
    return ret;
}

int main(int argc, char const *argv[]) {
#if TARGET_OS_WIN
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc != 5) {
        return EXIT_FAILURE;
    }
    int ret = 0;
    char const *CC = getenv("CC");
    char const *silis = argv[1];
    char const *target = "c";
    char const *dir_in = argv[2];
    char const *dir_out = argv[3];
    char const *test = argv[4];
    char *cwd = getcwd_full();
    printf("CWD=%s\n", cwd);
    free(cwd);
    printf("CC=%s\n", CC);
    printf("silis=%s\n", silis);
    printf("dir_in=%s\n", dir_in);
    printf("dir_out=%s\n", dir_out);
    printf("test=%s\n", test);
    run_or_exit(silis, target, dir_in, dir_out, test);
    char const *exe = va(TARGET_OS_WIN ? "%s\\%s.exe" : "%s/%s.exe", dir_out, test);
#if TARGET_OS_WIN
    run_or_exit(CC, "/nologo", va("%s\\%s.c", dir_out, test), "/Fe:", exe);
#else
    run_or_exit(CC, va("%s/%s.c", dir_out, test), "-o", exe);
#endif
    run_or_exit(exe);
    return EXIT_SUCCESS;
}

#if TARGET_OS_WIN

static wchar_t *u8_to_u16(size_t len, char const *str) {
    size_t wlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);
    wchar_t *wstr = calloc(wlen + 1, sizeof(*wstr));
    wstr[wlen] = '\0';
    MultiByteToWideChar(CP_UTF8, 0, str, len, wstr, wlen);
    return wstr;
}

static char *u16_to_u8(size_t wlen, wchar_t const *wstr) {
    size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, NULL, 0, NULL, NULL);
    char *str = calloc(len + 1, sizeof(*str));
    str[len] = '\0';
    WideCharToMultiByte(CP_UTF8, 0, wstr, wlen, str, len, NULL, NULL);
    return str;
}

static char *getcwd_full(void) {
    DWORD wcwdLength = GetCurrentDirectoryW(0, NULL);
    wchar_t *wcwd = calloc(wcwdLength, sizeof(*wcwd));
    GetCurrentDirectoryW(wcwdLength, wcwd);
    char *cwd = u16_to_u8(wcwdLength, wcwd);
    free(wcwd);
    return cwd;
}

typedef struct {
    void *memory;
    char *begin;
    size_t size;
} Argument;

static Argument QuoteArgument(size_t len, char const *arg) {
    size_t maxSize = /* '"' */ 1 + /* "\\" */ (len * 2) + /* '"' */ 1 + /* '\0' */ 1;
    char *memory = calloc(maxSize, sizeof(*memory));
    char *out = memory;
    *out++ = '"';
    char *str = out;
    BOOL unsafe = FALSE;
#define yield(c, n) do { for (int i = 0; i < (n); ++i) *out++ = (c); } while (0)
    for (char const *it = arg;; ++it) {
        int backslashes = 0;
        for (; *it == '\\'; ++it) {
            ++backslashes;
        }
        char c = *it;
        switch (c) {
            case '\0': {
                yield('\\', backslashes * 2);
                if (unsafe) {
                    str = memory;
                    *out++ = '"';
                }
                *out = '\0';
                const size_t size = out - str;
                return (Argument) {
                        .memory = memory,
                        .begin = str,
                        .size = size,
                };
            }
            case ' ':
            case '\t':
            case '\n':
            case '\v':
                unsafe = TRUE;
            default: {
                yield('\\', backslashes);
                yield(c, 1);
                break;
            }
            case '"': {
                unsafe = TRUE;
                yield('\\', backslashes * 2 + 1);
                yield('"', 1);
                break;
            }
        }
    }
#undef yield
}

int posix_spawn(
        pid_t *pid,
        const char *path,
        const posix_spawn_file_actions_t *file_actions,
        const posix_spawnattr_t *attrp,
        const char *argv[],
        const char *envp[]
) {
    (void) path;
    (void) file_actions;
    (void) attrp;
    (void) envp;
    size_t argc = 0;
    for (char const **arg = argv; *arg; ++arg, ++argc);
    size_t *argvLength = calloc(argc, sizeof(*argvLength));
    for (size_t i = 0; i < argc; ++i) {
        argvLength[i] = strlen(argv[i]);
    }

    Argument *arguments = calloc(argc, sizeof(*arguments));
    size_t length = 0;
    for (size_t i = 0; i < argc; ++i) {
        Argument a = arguments[i] = QuoteArgument(argvLength[i], argv[i]);
        length += a.size;
    }
    const size_t cmdlineLen = length + argc;
    char *cmdline = calloc(cmdlineLen, sizeof(*cmdline));
    char *out = cmdline;
    for (size_t i = 0; i < argc; ++i) {
        if (i) *out++ = ' ';
        Argument a = arguments[i];
        memcpy(out, a.begin, a.size);
        out += a.size;
        free(a.memory);
    }
    *out = '\0';
    wchar_t *wcmdline = u8_to_u16(cmdlineLen, cmdline);
    free(cmdline);
    STARTUPINFOW startupInfo = {
            .cb = sizeof(startupInfo),
            .dwFlags = STARTF_USESTDHANDLES,
            .hStdInput = GetStdHandle(STD_INPUT_HANDLE),
            .hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE),
            .hStdError = GetStdHandle(STD_ERROR_HANDLE),
    };
    PROCESS_INFORMATION proc = {0};
    BOOL ret = CreateProcessW(NULL, wcmdline, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &proc);
    free(wcmdline);
    if (!ret) {
        return 127;
    }
    if (pid) *pid = proc;
    return 0;
}

pid_t waitpid(pid_t pid, int *status, int options) {
    (void) options;
    WaitForSingleObject(pid.hProcess, INFINITE);
    if (status) {
        DWORD ret;
        if (GetExitCodeProcess(pid.hProcess, &ret)) {
            *status = ret;
        } else {
            *status = STILL_ACTIVE;
        }
    }
    return pid;
}

#else

static char *getcwd_full(void) {
    return strdup(getenv("PWD"));
}

#endif
