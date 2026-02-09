#include <new>
#include <stdlib.h>
#include "memory.h"

namespace snp {


// TODO Student
  void* Memory::_new(size_t size) {
    void* ptr = Memory::malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
  }

// TODO Student
  void Memory::_delete(void* p) {
    Memory::free(p);
  }


}
