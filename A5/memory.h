#ifndef SNP_MEMORY_H_
#define SNP_MEMORY_H_

#include "unistd.h"
#include "stdlib.h"

class Block {
  private:
  size_t canary_start_;
  size_t size_;
  bool free_;
  Block *next_;

  public:
  void setSize(size_t size) { size_ = size; }
  size_t getSize() { return size_; }
  void setFree(bool state) { free_ = state; }
  bool isFree() { return free_; }
  void setNext(Block* next) { next_ = next; }
  Block *getNext() { return next_; }
  void setCanaryStart(size_t canary) { canary_start_ = canary; }
  size_t getCanaryStart() { return canary_start_; }

};

namespace snp {
  class Memory
  {
  public:
    // DO NOT MODIFY
    static void *calloc(size_t num_memb, size_t size_each);
    static void *malloc(size_t size);
    static void free(void *ptr);

    // DO NOT MODIFY
    static void *_new(size_t size);
    static void _delete(void *ptr);

    static size_t total_alloc_memory() noexcept;
    static size_t block_info(int type) noexcept;

    // Feel free to add new members to this class.

  private:
    //*****************************
    // returns the start of the heap = sbrk(0)
    static void* heap_start_;

    //*****************************
    // points to the first block in the heap
    static Block* heap_list_;

    //*****************************
    // returns the total amount of allocated memory
    static size_t total_alloc_memory_;

  public:
    static void setHeapStart(void* start) { heap_start_ = start; }
    static void* getHeapStart() { return heap_start_; }
    static void setHeapList(Block* block) { heap_list_ = block; }
    static Block* getHeapList() { return heap_list_; }

    static Block* getFreeBlock(size_t size);
    static Block* allocateBlock(Block* last_block, size_t size);
    static void setTotalAllocMemory(size_t size) { total_alloc_memory_ = size; }
    static void incrementTotalAllocMemory(size_t size) { total_alloc_memory_ += size; }
    static size_t getTotalAllocMemory() { return total_alloc_memory_; }
    static void coalescing();
    static bool validatePointer(void* ptr);
    static Block* getLastHeapBlock();
    static void trimLastBlock(Block* last_block, Block* prev_block);
    static Block* firstBlock(size_t size);
    static Block* fillBestFit(size_t size, Block* block);
    static Block* mergeBlocks(Block* f_b, Block* s_b);
    static Block* findBestFit(size_t size);
    static Block* freeLastBlock(Block* last_block, size_t req_size);
  };

  // DO NOT MODIFY
  void* sbrk(intptr_t __delta);
  int brk(void* __addr);
}



// DO NOT CHANGE ANYTHING BELOW THIS LINE
void *malloc(size_t size) noexcept;
void *calloc(size_t num_memb, size_t size_each) noexcept;
void free(void *ptr) noexcept;
void* operator new(size_t size);
void operator delete(void *address ) noexcept;
void* operator new[] ( size_t size );
void operator delete[] ( void* address ) noexcept;

#endif /* SNP_MEMORY_H_ */
