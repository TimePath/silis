#include <cstdio>
#include <cstring>
#include <cstdlib>

int main(int argc, char const **argv) {
    argv += 1, argc -= 1;
    setbuf(stdout, nullptr);
    if (argc == 0) {
        printf("One\n");
        printf("DISABLED_Two\n");
        printf("Three\n");
        return 0;
    }
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    if (0) {
    } else if (strcmp(argv[0], "One") == 0) {
        printf("One\n");
    } else if (strcmp(argv[0], "Two") == 0) {
        printf("Two\n");
    } else {
        abort();
    }
    return 0;
}
