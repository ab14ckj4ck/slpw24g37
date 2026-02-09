//
// Created by florian on 28.10.24.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "extern.h"

#define ITERATIONS 1000000

int global_var = 0;
int shared_var = 0;

double howLong(void (*function)()) {
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    function();
    clock_gettime(CLOCK_REALTIME, &end);

    double seconds = end.tv_sec - start.tv_sec;
    double nanoseconds = end.tv_nsec - start.tv_nsec;

    return seconds + nanoseconds;
}

void stack() {
    int stack_var = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        stack_var++;
    }
}

void heap() {
    int *heap_var = malloc(sizeof(int));
    for (int i = 0; i < ITERATIONS; i++) {
        (*heap_var)++;
    }
    free(heap_var);
}

void global() {
    for (int i = 0; i < ITERATIONS; i++) {
        global_var++;
    }
}

void externAccess() {
    for (int i = 0; i < ITERATIONS; i++) {
        extern_var++;
    }
}

int main() {
    double stack_time = howLong(stack);
    double heap_time = howLong(heap);
    double global_time = howLong(global);
    double extern_time = howLong(externAccess);

    printf("Stack: %f\n", stack_time);
    printf("Heap: %f\n", heap_time);
    printf("Global: %f\n", global_time);
    printf("Extern: %f\n", extern_time);

    return 0;
}