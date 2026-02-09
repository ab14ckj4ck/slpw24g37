//
// Created by florian on 28.10.24.
//

// program shows just the addresses of local variables in threads
// but (!) those addresses are in separate stacks. Because every thread got its own stack.
// The segfault described in the question happenes because threads are not allowed to access the stack of another thread.
#include <stdio.h>
#include <pthread.h>

void* thread_function(void* arg) {
    int local_var;
    printf("Thread %ld: Adresse der lokalen Variable: %p\n", (long)arg, (void*)&local_var);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread_function, (void*)1);
    pthread_create(&thread2, NULL, thread_function, (void*)2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
