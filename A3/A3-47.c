//
// Created by florian on 27.10.24.
//
#include <stdio.h>
#include <stdlib.h>

long fileSize(const char *filename){
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

int main() {
    const char* filename = "a";
    long size = fileSize(filename);
    if (size == -1) {
        printf("File not found\n");
    } else {
        printf("Size of file %s: %ld bytes\n", filename, size);
    }

    int arr[10];

    int* ptr = (int*)malloc(10 * sizeof(int));
    if (ptr == NULL) {
        printf("Memory allocation failed\n");
    }

    size = fileSize(filename);
    if (size == -1) {
        printf("File not found\n");
    } else {
        printf("Size of file after array %s: %ld bytes\n", filename, size);
    }
}
