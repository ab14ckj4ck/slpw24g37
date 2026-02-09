#include <stdio.h>
#include <string.h>

int main() {
    FILE* file = fopen("/proc/self/status", "r");
    char line[256];
    int memory = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %d KB", &memory);
            break;
        }
    }

    fclose(file);
    printf("Current RAM usage: %d KB\n", memory);

}