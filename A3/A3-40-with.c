//
// Created by florian on 27.10.24.
//
#include <stdio.h>
#include <string.h>

char arr[1000000];

int main() {
    FILE* file = fopen("/proc/self/status", "r");
    char line[256];
    int memory = 0;

    for (size_t i = 0; i < sizeof(arr); i++) {
        arr[i] = 'a';
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %d KB", &memory);
            break;
        }
    }

    fclose(file);
    printf("Current RAM usage: %d KB\n", memory);

}