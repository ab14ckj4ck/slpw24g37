// This should have all the imports you need.
#include <pthread.h>
#include "memory.h"
#include <new>

namespace snp {
// TODO Student

  void* Memory::heap_start_ = sbrk(0);
  Block* Memory::heap_list_ = nullptr;
  size_t Memory::total_alloc_memory_ = 0;
  static pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;
  static pthread_mutex_t calloc_lock = PTHREAD_MUTEX_INITIALIZER;

  void* Memory::malloc(size_t size) {
    pthread_mutex_lock(&malloc_lock);

    Block* new_block = getFreeBlock(size);
    if (!new_block) {
      pthread_mutex_unlock(&malloc_lock);
      return nullptr;
    }

    incrementTotalAllocMemory(size);

    void* ptr_to_return = (void*) (new_block + 1);

    pthread_mutex_unlock(&malloc_lock);
    return ptr_to_return;
  }

  void Memory::free(void* ptr) {
    pthread_mutex_lock(&malloc_lock);z

    if (!ptr) {



      pthread_mutex_unlock(&malloc_lock);
      return;
    }

    if (!validatePointer(ptr)) {
      pthread_mutex_unlock(&malloc_lock);
      exit(-1);
    }

    Block* block_to_free = (Block*) ptr - 1;

    block_to_free->setFree(true);

    coalescing();

    pthread_mutex_unlock(&malloc_lock);
  }

  void* Memory::calloc(size_t num_memb, size_t size_each) {
    auto MAX_SIZE = (size_t)-1;

    pthread_mutex_lock(&calloc_lock);
    if (num_memb * size_each > MAX_SIZE) {
      pthread_mutex_unlock(&calloc_lock);
      return nullptr;
    }

    size_t total_size = num_memb * size_each;
    void* ptr = malloc(total_size);
    if (!ptr) {
      pthread_mutex_unlock(&calloc_lock);
      return nullptr;
    }

    for (size_t i = 0; i < total_size; ++i) {
      *((char*) ptr + i) = 0;
    }

    pthread_mutex_unlock(&calloc_lock);
    return ptr;
  }

  size_t Memory::total_alloc_memory() noexcept {
    return getTotalAllocMemory();
  }

  Block* Memory::getFreeBlock(size_t size) {
    Block* block_to_return = nullptr;
    if (!getHeapList()) {
      block_to_return = firstBlock(size);
      if (!block_to_return) return nullptr;

      return block_to_return;
    }

    //find a free block with enough size for new_block and filler block
    Block* best_fit = findBestFit(size);

    if (best_fit) {
      block_to_return = fillBestFit(size, best_fit);
      if (!block_to_return) return nullptr;
      return block_to_return;
    }

    Block* last_block = getLastHeapBlock();

    block_to_return = allocateBlock(last_block, size);
    if (!block_to_return) return nullptr;
    return block_to_return;
  }

  Block* Memory::findBestFit(size_t size) {
    Block* best_fit = nullptr;
    Block* current_block = getHeapList();

    while (current_block) {
      if (current_block->isFree() && current_block->getSize() >= size) {
        if (!best_fit || current_block->getSize() < best_fit->getSize()) {
          best_fit = current_block;
        }
      }
      current_block = current_block->getNext();
    }

    return best_fit;
  }

  Block* Memory::fillBestFit(size_t req_size, Block* best_fit_block) {
    size_t threshold = 64;
    size_t block_size = sizeof(Block);
    size_t canary_size = sizeof(size_t);
    size_t canary = 0xdeadbeef;

    if (!best_fit_block || best_fit_block->getSize() < req_size) return nullptr;
    if (best_fit_block->getCanaryStart() != canary) exit(-1);

    size_t current_block_size = best_fit_block->getSize();
    size_t remaining_usable_space = 0;

    if (current_block_size < req_size + canary_size + block_size + threshold) {
      remaining_usable_space = 0;
    } else {
      remaining_usable_space = current_block_size - req_size - canary_size - block_size;
    }

    // if the bestFit block has exactly the size of the req_size or the usable space of the filler_block is < threshold
    if (remaining_usable_space < threshold) {
      best_fit_block->setSize(current_block_size);
      best_fit_block->setFree(false);
      return best_fit_block;
    }

    // if the bestFit has enough space for a filler block with > threshold usable space
    size_t req_block_size = req_size + canary_size;
    Block* new_block = best_fit_block;
    Block* filler_block = nullptr;

    new_block->setSize(req_size);
    new_block->setFree(false);

    auto* new_block_canary_end = (size_t*) ((char*) new_block + block_size + req_size);
    *new_block_canary_end = canary;

    filler_block = (Block*) ((char*) new_block + block_size + req_block_size);
    filler_block->setCanaryStart(canary);
    filler_block->setSize(remaining_usable_space);
    filler_block->setFree(true);
    filler_block->setNext(new_block->getNext());

    auto* filler_block_canary_end = (size_t*) ((char*) filler_block + block_size + remaining_usable_space);
    *filler_block_canary_end = canary;

    new_block->setNext(filler_block);

    return new_block;
  }

  Block* Memory::allocateBlock(Block* last_block, size_t size) {
    size_t page = 4096;
    size_t canary = 0xdeadbeef;
    size_t threshold = 64;
    size_t canary_size = sizeof(size_t);
    size_t block_size = sizeof(Block);
    Block* final_block = nullptr;

    size_t total_size = ((block_size + size + canary_size + page - 1) / page) * page;
    size_t temp_block_size = total_size - block_size - canary_size;

    auto* temp_block = (Block*) sbrk((intptr_t) total_size);
    if (!temp_block || temp_block == (void*) -1) return nullptr;

    temp_block->setCanaryStart(canary);
    temp_block->setSize(temp_block_size);
    temp_block->setFree(true);
    temp_block->setNext(nullptr);

    auto* temp_block_canary_end = (size_t*) ((char*) temp_block + temp_block->getSize() + sizeof(Block));
    *temp_block_canary_end = canary;

    if (last_block && last_block->isFree()) {
      last_block->setNext(temp_block);
      final_block = mergeBlocks(last_block, temp_block);
    } else {
      if (last_block) last_block->setNext(temp_block);
      final_block = temp_block;
    }

    if (!final_block) return nullptr;

    if (final_block->getSize() > size + canary_size + block_size + threshold) {
      size_t remaining_size = final_block->getSize() - size - canary_size - block_size;
      auto* filler_block = (Block*) ((char*) final_block + block_size + size + canary_size);

      final_block->setSize(size);
      final_block->setFree(false);

      auto* final_block_canary_end = (size_t*) ((char*) final_block + block_size + size);
      *final_block_canary_end = canary;

      filler_block->setCanaryStart(canary);
      filler_block->setSize(remaining_size);
      filler_block->setFree(true);
      filler_block->setNext(nullptr);

      auto* filler_block_canary_end = (size_t*) ((char*) filler_block + block_size + remaining_size);
      *filler_block_canary_end = canary;

      final_block->setNext(filler_block);
    } else {
      if (last_block) last_block->setNext(final_block);
      final_block->setFree(false);
    }

    return final_block;
  }

/*
  Block* Memory::allocateBlock(Block* last_block, size_t size) {
    size_t page = 4096;
    size_t canary = 0xdeadbeef;
    size_t threshold = 64;
    size_t canary_size = sizeof(size_t);
    size_t block_size = sizeof(Block);
    size_t total_size = ((block_size + size + canary_size + page - 1) / page) * page;

    if (size == 0) total_size = 0 + block_size + canary_size;

    if (last_block && last_block->isFree()) {
      total_size -= last_block->getSize();
    }

    auto* new_block = (Block*) sbrk((intptr_t) total_size);
    if (!new_block || new_block == (void*) -1) return nullptr;

    new_block->setCanaryStart(canary);
    new_block->setSize(size);
    new_block->setFree(false);
    new_block->setNext(nullptr);

    if (last_block && last_block->isFree()) {
      mergeBlocks(last_block, new_block);
    } else {
      auto* new_block_canary_end = (size_t*) ((char*) new_block + block_size + size);
      *new_block_canary_end = canary;
    }

    size_t remaining_size = 0;
    if (total_size > block_size + size + canary_size + block_size + threshold + canary_size) {
      remaining_size = total_size - block_size - size - canary_size;
    } else {
      remaining_size = 0;
    }

    if (remaining_size == 0) {
      return new_block;
    }

    size_t filler_block_size = remaining_size - block_size - canary_size;
    auto* filler_block = (Block*) ((char*) new_block + block_size + size + canary_size);

    filler_block->setCanaryStart(canary);
    filler_block->setSize(filler_block_size);
    filler_block->setFree(true);
    filler_block->setNext(nullptr);
    new_block->setNext(filler_block);

    auto* filler_block_canary_end = (size_t*) ((char*) filler_block + block_size + filler_block_size);
    *filler_block_canary_end = canary;

    return new_block;
  }
*/

  Block* Memory::firstBlock(size_t size) {
    Block* new_block = allocateBlock(nullptr, size);
    if (!new_block) return nullptr;

    setHeapList(new_block);
    if (!getHeapList()) return nullptr;

    return new_block;
  }

  bool Memory::validatePointer(void* ptr) {
    if (!getHeapList()) return false;
    Block* current_block = getHeapList();

    while (current_block) {
      if (current_block == (Block*) ptr - 1) {
        if (current_block->getCanaryStart() != 0xdeadbeef) return false;
        if (current_block->isFree()) return false;

        auto* canary_end = (size_t*) ((char*) current_block + sizeof(Block) + current_block->getSize());
        if (*canary_end != 0xdeadbeef) return false;

        return true;
      }

      current_block = current_block->getNext();
    }

    return false;
  }

  void Memory::coalescing() {
    Block* prev_block = nullptr;
    Block* current_block = getHeapList();

    while (current_block) {
      Block* next_block = current_block->getNext();

      if (!next_block) break;

      if (current_block->isFree() && next_block->isFree()) {
        Block* merged_block = mergeBlocks(current_block, next_block);
        if (merged_block) {
          current_block = merged_block;
          continue;
        }
      }
      prev_block = current_block;
      current_block = next_block;
    }

    if (!current_block) return;

    if (current_block->isFree() && current_block == getHeapList()) {
      sbrk(-(intptr_t) (current_block->getSize() + sizeof(Block) + sizeof(size_t)));
      setHeapList(nullptr);
      return;
    }

    trimLastBlock(current_block, prev_block);
  }

  Block* Memory::mergeBlocks(Block* f_b, Block* s_b) {

    if (!f_b || !s_b) return nullptr;
    if (f_b->getCanaryStart() != 0xdeadbeef || s_b->getCanaryStart() != 0xdeadbeef) exit(-1);
    auto* f_b_canary_end = (size_t*) ((char*) f_b + f_b->getSize() + sizeof(Block));
    auto* s_b_canary_end = (size_t*) ((char*) s_b + s_b->getSize() + sizeof(Block));
    if (*f_b_canary_end != 0xdeadbeef || *s_b_canary_end != 0xdeadbeef) exit(-1);

    Block* merged_block = nullptr;
    size_t canary = 0xdeadbeef;
    size_t block_size = sizeof(Block);
    size_t canary_size = sizeof(size_t);

    size_t new_user_size = block_size + f_b->getSize() + canary_size + s_b->getSize();
    merged_block = f_b;
    merged_block->setSize(new_user_size);
    merged_block->setFree(true);
    merged_block->setNext(s_b->getNext());

    auto* merged_block_canary_end = (size_t*) ((char*) merged_block + block_size + new_user_size);
    *merged_block_canary_end = canary;

    return merged_block;
  }

  void Memory::trimLastBlock(Block* last_block, Block* prev_block) {
    if (!last_block) return;
    if (last_block->getNext()) return;
    if (!last_block->isFree()) return;

    size_t size_to_trim = last_block->getSize() + sizeof(Block) + sizeof(size_t);

    auto* return_message = sbrk(-(intptr_t) size_to_trim);
    if (return_message == (void*) -1) exit(-1);

    if (prev_block) {
      prev_block->setNext(nullptr);
    } else {
      setHeapList(nullptr);
    }
  }

  Block* Memory::getLastHeapBlock() {
    Block* current_block = getHeapList();
    if (!current_block) return nullptr;

    while (current_block->getNext()) {
      current_block = current_block->getNext();
    }
    return current_block;
  }

  size_t Memory::block_info(int type) noexcept {
    pthread_mutex_lock(&malloc_lock);

    Block* current_block = getHeapList();

    while (current_block) {
      if (!current_block->isFree()) {
        if (type == 0) {
          pthread_mutex_unlock(&malloc_lock);
          return (size_t) (current_block + 1);
        } else {
          pthread_mutex_unlock(&malloc_lock);
          return current_block->getSize();
        }
      }
      current_block = current_block->getNext();
    }

    pthread_mutex_unlock(&malloc_lock);
    return 0;
  }


// TODO Student End
}
