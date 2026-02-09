//
// Created by florian on 28.10.24.
//
// Memory is not instantly aquired when useing fork(). OS works with copy-on-write (COW), so the memory is only copied when it is written to (bigger changes)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void print_memory_usage(const char *label) {
    char filepath[256];
    FILE *fp;
    char line[128];

    snprintf(filepath, sizeof(filepath), "/proc/%d/status", getpid());
    fp = fopen(filepath, "r");

    printf("\n%s - PID: %d\n", label, getpid());
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmRSS", 5) == 0) {
            printf("%s", line);
            break;
        }
    }
    fclose(fp);
}

int main() {
    int *data = malloc(1024 * 1024 * sizeof(int)); //

    for (int i = 0; i < 1024 * 1024; i++) {
        data[i] = i;
    }

    print_memory_usage("Before fork");

    pid_t pid = fork();

    if (pid == 0) {
        print_memory_usage("Child process (before modification)");
        data[0] = 2024;
        print_memory_usage("Child process (after modification)");
        free(data);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
        print_memory_usage("Parent process (after child exit)");
        free(data);
    }

    return EXIT_SUCCESS;
}
