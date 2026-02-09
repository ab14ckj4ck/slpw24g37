#include "malloc.cpp"

// DO NOT PUSH CHANGES IN THIS FILE
// You can change it locally, if you want to add new features for your own tests

void* snp::sbrk(intptr_t __delta)
{
  return ::sbrk(__delta);
}

int snp::brk(void* __addr)
{
  return ::brk(__addr);
}

void *calloc(size_t num_memb, size_t size_each) noexcept
{
  return snp::Memory::calloc(num_memb, size_each);
}

void *malloc(size_t size) noexcept
{
  return snp::Memory::malloc(size);
}

void free(void *ptr) noexcept
{
  snp::Memory::free(ptr);
}

extern "C" size_t total_alloc_memory()
{
  return snp::Memory::total_alloc_memory();
}

extern "C" size_t block_info(int type)
{
  return snp::Memory::block_info(type);
}

// If you want to add your own test functions and call them in your tests,
// adapt this template to your new functions.
// Be careful: The testsystem will not use your util.cpp
/*
extern "C" size_t your_test_function()
{
  return snp::Memory::your_test_function();
}
*/

#ifdef NEW_ENABLED

void* operator new(size_t size)
{
  return snp::Memory::_new(size);
}

void operator delete(void *address )
{
  snp::Memory::_delete(address);
}

void operator delete(void* address, std::size_t size)
{
  (void) size;
  snp::Memory::_delete(address);
}

void* operator new[] ( size_t size )
{
  return snp::Memory::_new(size);
}

void operator delete[] ( void* address )
{
  snp::Memory::_delete(address);
}

void operator delete [](void* address, std::size_t size)
{
  (void) size;
  snp::Memory::_delete(address);
}

#endif
