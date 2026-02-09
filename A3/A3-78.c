    //
    // Created by florian on 28.10.24.
    //

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/mman.h>

    void writeHeap(int* ptr) {
        *ptr = 10;
    }

    void writeMmap(int* ptr) {
        *ptr = 10;
    }

    void writeFunc(int* ptr) {
        *ptr = 10;
    }

    int main () {
        //tries to write to freed memory
        int* heap_ptr = (int*) malloc(sizeof(int));
        *heap_ptr = 5;
        free(heap_ptr);
        writeHeap(heap_ptr);

        //tries to write to munmaped memory (munmaped memory is like freed memory)
        size_t size = sysconf(_SC_PAGESIZE);
        int* mmap_ptr = (int*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *mmap_ptr = 5;
        munmap(mmap_ptr, size);
        writeMmap(mmap_ptr);

        //tries to write to a function (functions only have read and execute permissions)
        int* fun_ptr = (int*) &writeHeap;
        writeFunc(fun_ptr);



        return 0;
    }