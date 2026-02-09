//
// Created by florian on 27.10.24.
//
// heap is right above data segment (where global variables are stored)

#include <stdio.h>
#include <stdlib.h>

int global = 10;

int main() {
    int* ptr = (int*) malloc(sizeof(int));
    printf("global: %p\n", &global);
    printf("heap  : %p\n", ptr);
    free(ptr);
    return 0;
}