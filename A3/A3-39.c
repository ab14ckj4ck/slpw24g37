//
// Created by florian on 26.10.24.
// 0x5 / 0x6 because of the size. System uses so small blocks of memory, to fill small "holes" in the memory. Even though it is on the heap.
//

#include <stdio.h>
#include <stdlib.h>

int main() {
    size_t size = 64 * 1024; // 64 KB
    void *ptr = malloc(size);

    if (ptr == NULL) {
        printf("Failed to allocate memory\n");
        return 1;
    } else {
        printf("Allocated %lu bytes of memory\n", size);
        printf("%p\n", ptr);
        free(ptr);
        return 0;
    }
}