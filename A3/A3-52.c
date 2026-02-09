//
// Created by florian on 27.10.24.
//
// Try to dereference a NULL pointer. -> Try to access variable behind a pointer, but the pointer doesn't point to that variable but to NULL. Could happen after Pointer = NULL, or failed alloc. or by coincidence after free in form of undefined behavior.
// valgrind ./a

#include <stdio.h>
#include <stdlib.h>

int main() {
    int* arr = NULL;
    arr[6] = 7;
    return 0;
}
