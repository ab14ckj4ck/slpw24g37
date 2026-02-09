//
// Created by florian on 26.10.24.
//
// should be 0x7 because the memory is allocated on the heap
//

#include <stdio.h>
#include <stdlib.h>

int main() {
    size_t size = 512 * 1024 * 1024;
    void *ptr = malloc(size);

    if (ptr == NULL) {
        printf("Failed to allocate memory\n");
        return 1;
    } else {
        printf("Successfully allocated %zu bytes\n", size);
        printf("%p", ptr);
        free(ptr);
        return 0;
    }

    return 0;
}

0x6
0x7