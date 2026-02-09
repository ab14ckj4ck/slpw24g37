#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "A3-43-helper.h"

//clang -o a A3-43.c A3-43-helper.c

int global_variable = 0;
int shared_variable = 0;

void countUp(int a) {
    for (int i = 0; i < a; i++) {
        shared_variable++;
    }
}

void function() {
    printf("Hello, World!\n");
}

int main() {
    int stack_variable = 0;
    static int static_variable = 1;
    pthread_t thread_1, thread_2;
    int* heap_variable = (int*)malloc(sizeof(int));

    pthread_create(&thread_1, NULL, (void *(*)(void *)) countUp, NULL);
    pthread_create(&thread_2, NULL, (void *(*)(void *)) countUp, NULL);

    printf("Address of countUp: %p\n", (void*)countUp);
    printf("Address of function: %p\n", (void*)function);
    printf("\n\n");
    printf("Address of extern global variable: %p\n", &extern_var);
    printf("Address of global variable: %p\n", &global_variable);
    printf("Address of shared variable: %p\n", &shared_variable);
    printf("Address of static variable: %p\n", &static_variable);
    printf("Address of heap variable: %p\n", heap_variable);
    printf("Address of stack variable: %p\n", &stack_variable);

    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);

    free(heap_variable);
    return 0;
}
