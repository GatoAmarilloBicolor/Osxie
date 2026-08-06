#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "git") == 0) {
        execvp("git", &argv[1]);
    } else if (argc > 1) {
        execvp(argv[1], &argv[1]);
    }
    fprintf(stderr, "xcrun: missing arguments\n");
    return 1;
}
