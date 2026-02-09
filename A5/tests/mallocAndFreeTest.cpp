#include "../memory.h"
#include <pthread.h>

#define SUCCESS 0

#define MALLOC_FAILED 11
#define SIZE_MISMATCH 12
#define FREE_FAILED 13
#define ASSIGN_FAILED 14
#define CANARY_START_FAILED 15
#define CANARY_END_FAILED 16
#define MERGING_FAILED 17
#define SMALL_MALLOC_FAILED 18
#define SMALL_FREE_FAILED 19
#define BRK_VAL_FAILED 20
#define BIG_MALLOC_FAILED 21
#define FRAG_FAILED 22
#define TRIM_FAILED 23
#define THREAD_MALLOC_FAILED 24
#define THREAD_FREE_FAILED 25
#define TOTAL_BRK_VAL_FAILED 26
#define ASC_MALLOC_FAILED 27
#define ASC_FREE_FAILED 28
#define ASC_ASSIGN_FAILED 29
#define DSC_MALLOC_FAILED 30
#define DSC_FREE_FAILED 31
#define DSC_ASSIGN_FAILED 32
#define BRK_VAL_TRIM_FAILED 33

#define TEST_BRK_C 40
#define TEST_BRK_A 41
#define TEST_BRK_BEGINNING 42

#define TOTAL_ALLOC_SIZE 10
#define TOTAL_THREAD_NUM 20

#define assert(val) do {\
 if ((val) == 0) exit(100);\
} while (0);

void retCode(int return_code) {
  if (return_code != 0) exit(return_code);
}

int mallocALot() {

  int* ptrs[TOTAL_ALLOC_SIZE];
  size_t size = 0;
  size_t control_size = 0;

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {

    ptrs[i] = (int*) malloc(sizeof(int));
    if (!ptrs[i]) {
      return MALLOC_FAILED;
    }
    Block* block = (Block*) ptrs[i] - 1;
    if (block->getCanaryStart() != 0xdeadbeef) {
      return CANARY_START_FAILED;
    }
    if (i < TOTAL_ALLOC_SIZE - 1) size += block->getSize();

    auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
    if (*canary_end != 0xdeadbeef) {
      return CANARY_END_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    Block* block = (Block*) ptrs[i] - 1;
    if (block->getCanaryStart() != 0xdeadbeef) {
      return CANARY_START_FAILED;
    }
    *ptrs[i] = i;
    if (*ptrs[i] != i) {
      return ASSIGN_FAILED;
    }

    auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
    if (*canary_end != 0xdeadbeef) {
      return CANARY_END_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    Block* block = (Block*) ptrs[i] - 1;
    if (block->getCanaryStart() != 0xdeadbeef) {
      return CANARY_START_FAILED;
    }

    auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
    if (*canary_end != 0xdeadbeef) {
      return CANARY_END_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    if (i < TOTAL_ALLOC_SIZE - 1) {
      Block* block = (Block*) ptrs[i] - 1;
      control_size += block->getSize();
    }
    free(ptrs[i]);
  }

  if (control_size != size) {
    return SIZE_MISMATCH;
  }

  if (snp::Memory::getHeapList()) {
    return FREE_FAILED;
  }

  return SUCCESS;
}

int checkMerging() {
  int* ptrs[TOTAL_ALLOC_SIZE];
  size_t control_size = 0;

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    ptrs[i] = (int*) malloc(sizeof(int));
    if (!ptrs[i]) {
      return MALLOC_FAILED;
    }
    Block* block = (Block*) ptrs[i] - 1;

    if (block->getCanaryStart() != 0xdeadbeef) {
      return CANARY_START_FAILED;
    }

    if (i < TOTAL_ALLOC_SIZE - 1) control_size += block->getSize();

    auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
    if (*canary_end != 0xdeadbeef) {
      return CANARY_END_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i += 2) {
    free(ptrs[i]);
  }

  Block* current_block = snp::Memory::getHeapList();
  while (current_block) {
    Block* next_block = current_block->getNext();

    if (current_block->isFree() && next_block && next_block->isFree()) {
      return MERGING_FAILED;
    }

    current_block = next_block;
  }

  for (int i = 1; i < TOTAL_ALLOC_SIZE; i += 2) {
    free(ptrs[i]);
  }

  return SUCCESS;
}

int smallMalloc() {
  auto* brk_val = sbrk(0);
  void* ptr = malloc(0);
  if (!ptr) {
    return SMALL_MALLOC_FAILED;
  }

  auto* brk_val2 = sbrk(0);

  Block* heap = snp::Memory::getHeapList();

  free(ptr);
  heap = snp::Memory::getHeapList();
  if (heap) {
    return SMALL_FREE_FAILED;
  }

  auto* brk_val3 = sbrk(0);

  return SUCCESS;
}

int testMallocOneByte() {
  Block* block = snp::Memory::getHeapList();
  void* ptr = malloc(1);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  block = (Block*) ptr - 1;
  if (block->getCanaryStart() != 0xdeadbeef) {
    return CANARY_START_FAILED;
  }

  auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
  if (*canary_end != 0xdeadbeef) {
    return CANARY_END_FAILED;
  }

  free(ptr);
  return SUCCESS;
}

int testBreak() {
  int* ptr[TOTAL_ALLOC_SIZE];
  auto bkr_val = sbrk(0);

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    ptr[i] = (int*) malloc(sizeof(int));
    if (!ptr[i]) {
      return MALLOC_FAILED;
    }
  }


  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    *ptr[i] = i;
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    if (*ptr[i] != i) {
      return ASSIGN_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    free(ptr[i]);
  }

  auto bkr_val2 = sbrk(0);

  if (bkr_val != bkr_val2) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int testBrk2()
{
  void* start = sbrk(0);

  void* a = malloc(100);
  void* b = malloc(1000000);
  void* c = malloc(0);
  void* d = malloc(1000);

  auto* current_brk = sbrk(0);

  free(d);

  current_brk = sbrk(0);

  if (current_brk != ((char*) c + sizeof(size_t))) return TEST_BRK_C;

  free(b);
  free(c);

  current_brk = sbrk(0);

  if (current_brk != ((char*) a + 100 + sizeof(size_t))) return TEST_BRK_A;

  free(a);

  current_brk = sbrk(0);

  if (current_brk != start)
    return TEST_BRK_BEGINNING;

  return SUCCESS;
}

int nulllPtr() {
  free(nullptr);
  return SUCCESS;
}

int testLargeAllocation() {
  size_t large_size = 1024 * 1024 * 1024; // 1 GiB
  void* ptr = malloc(large_size);
  if (!ptr) {
    return BIG_MALLOC_FAILED;
  }
  free(ptr);
  return SUCCESS;
}

int testFragmentation() {
  void* ptr1 = malloc(200);
  void* ptr2 = malloc(300);
  void* ptr3 = malloc(400);
  free(ptr2); // Fragment in der Mitte

  void* ptr4 = malloc(250); // Sollte den freigegebenen Block aufteilen
  if (!ptr4) {
    return FRAG_FAILED;
  }

  free(ptr1);
  free(ptr3);
  free(ptr4);

  return SUCCESS;
}

int testCoalescing() {
  void* ptr1 = malloc(100);
  void* ptr2 = malloc(200);
  void* ptr3 = malloc(300);
  Block* heap = snp::Memory::getHeapList();
  free(ptr1);
  free(ptr2);

  heap = snp::Memory::getHeapList();

  if (heap->getSize() != 340) {
    return MERGING_FAILED;
  }

  free(ptr3);
  return SUCCESS;
}

int testTrimLastBlock() {
  Block* heap = snp::Memory::getHeapList();
  void* ptr1 = malloc(2500);
  heap = snp::Memory::getHeapList();
  void* ptr2 = malloc(2700);
  heap = snp::Memory::getHeapList();


  free(ptr2);

  heap = snp::Memory::getHeapList();
  while (heap->getNext()) {
    heap = heap->getNext();
  }

  if (heap->getSize() > 4096) {
    free(ptr1);
    return TRIM_FAILED;
  }

  free(ptr1);

  return SUCCESS;
}

int testRandomAllocations() {
  void* ptrs[TOTAL_ALLOC_SIZE];

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    ptrs[i] = malloc(rand() % TOTAL_ALLOC_SIZE + 1);
    if (!ptrs[i]) {
      for (int j = 0; j < i; ++j) {
        free(ptrs[j]);
      }
      return MALLOC_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; ++i) {
    int index;
    do {
      index = rand() % TOTAL_ALLOC_SIZE;
    } while (ptrs[index] == NULL);

    free(ptrs[index]);
    ptrs[index] = NULL;
  }

  return SUCCESS;
}

void* threadFunc(void* arg) {
  (void)arg;
  int* ptr = (int*) malloc(sizeof(int));
  *ptr = 1;
  free(ptr);
  return NULL;
}

int testThreadSafety() {
  Block* heap = snp::Memory::getHeapList();
  pthread_t threads[TOTAL_THREAD_NUM];

  for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
    auto return_code = pthread_create(&threads[i], NULL, threadFunc, NULL);
    if (return_code != 0) {
      return THREAD_MALLOC_FAILED;
    }
  }

  heap = snp::Memory::getHeapList();

  for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
    pthread_join(threads[i], NULL);
  }

  heap = snp::Memory::getHeapList();
  if (heap) {
    return THREAD_FREE_FAILED;
  }

  return SUCCESS;
}

int testSmallAllocation() {
  void* ptr = malloc(1);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  Block* block = (Block*) ptr - 1;
  if (block->getSize() < 1) {
    free(ptr);
    return SIZE_MISMATCH;
  }

  free(ptr);
  return SUCCESS;
}

int testSmallFree() {
  void* ptr = malloc(1);
  if (!ptr) {
    return SMALL_MALLOC_FAILED;
  }

  Block* block = (Block*) ptr - 1;

  auto* canary_end = (size_t*) ((char*) block + sizeof(Block) + block->getSize());
  if (*canary_end != 0xdeadbeef) {
    return CANARY_END_FAILED;
  }

  free(ptr);

  Block* heap = snp::Memory::getHeapList();
  if (heap) {
    return SMALL_FREE_FAILED;
  }

  return SUCCESS;
}

int ascendingMalloc() {
  int* ptr[TOTAL_ALLOC_SIZE];

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    ptr[i] = (int*) malloc((i + 1) * sizeof(int));
    if (!ptr[i]) {
      return ASC_MALLOC_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    for (int j = 0; j <= i; j++) {
      ptr[i][j] = i;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    for (int j = 0; j <= i; j++) {
      if (ptr[i][j] != i) {
        return ASC_ASSIGN_FAILED;
      }
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    free(ptr[i]);
  }

  Block* heap_ = snp::Memory::getHeapList();
  if (heap_ != nullptr) {
    return ASC_FREE_FAILED;
  }

  return SUCCESS;
}

int descendingMalloc() {
  int* ptr[TOTAL_ALLOC_SIZE];

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    ptr[i] = (int*) malloc((TOTAL_ALLOC_SIZE - i) * sizeof(int));
    if (!ptr[i]) {
      return DSC_MALLOC_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    for (int j = 0; j < (TOTAL_ALLOC_SIZE - i); j++) {
      ptr[i][j] = i;
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    for (int j = 0; j < (TOTAL_ALLOC_SIZE - i); j++) {
      if (ptr[i][j] != i) {
        return DSC_ASSIGN_FAILED;
      }
    }
  }

  for (int i = 0; i < TOTAL_ALLOC_SIZE; i++) {
    free(ptr[i]);
  }

  Block* heap_ = snp::Memory::getHeapList();
  if (heap_ != nullptr) {
    return DSC_FREE_FAILED;
  }

  return SUCCESS;
}

int mallocZeroInbetween() {

  void* brk_val = sbrk(0);
  void* ptr1 = malloc(100);
  void* ptr2 = malloc(0);
  void* ptr3 = malloc(200);

  if (!ptr1 || !ptr2 || !ptr3) {
    return MALLOC_FAILED;
  }

  free(ptr1);
  free(ptr2);
  free(ptr3);

  Block* heap = snp::Memory::getHeapList();
  if (heap) {
    return FREE_FAILED;
  }

  void* ptr5 = malloc(100);
  void* ptr6 = malloc(200);
  void* ptr7 = malloc(100);

  free(ptr6);

  void* ptr8 = malloc(0);
  if (!ptr8) {
    return MALLOC_FAILED;
  }

  free(ptr5);
  free(ptr7);
  free(ptr8);

  heap = snp::Memory::getHeapList();
  if (heap) {
    return FREE_FAILED;
  }
  void* brk_val2 = sbrk(0);

  if (brk_val != brk_val2) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int noChangeBrk() {
  void* brk_val = sbrk(0);
  if (brk_val == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  if (brk_val != current_brk) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int brkExtension() {
  void* brk_val = sbrk(0);
  void* ptr = malloc(4096);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  if (brk_val == current_brk) {
    return BRK_VAL_FAILED;
  }

  free(ptr);

  return SUCCESS;
}

int brkReduction() {
  void* brk_val = sbrk(0);
  void* ptr = malloc(4096);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  if (brk_val == current_brk) {
    return BRK_VAL_FAILED;
  }

  free(ptr);

  return SUCCESS;
}

int fragBrk() {
  void* brk_val = sbrk(0);
  void* ptr1 = malloc(100);
  void* ptr2 = malloc(200);
  void* ptr3 = malloc(300);

  void* brk_val_after = sbrk(0);

  free(ptr2);

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  free(ptr1);
  free(ptr3);

  if (sbrk(0) != brk_val) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int testBrkMultipleExtensions() {
  void* initial_brk = sbrk(0);
  void* ptr1 = malloc(1000);
  void* ptr2 = malloc(2000);
  void* ptr3 = malloc(3000);

  if (!ptr1 || !ptr2 || !ptr3) {
    return MALLOC_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk <= initial_brk) {
    return BRK_VAL_FAILED;
  }

  free(ptr1);
  free(ptr2);
  free(ptr3);

  void* final_brk = sbrk(0);
  if (final_brk != initial_brk) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int rlyBigAllocBrk() {
  size_t size = 1024 * 1024 * 1024; // 1 GiB
  void* brk_val = sbrk(0);
  void* ptr = malloc(size);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }
  if (brk_val == current_brk) {
    return BRK_VAL_FAILED;
  }

  free(ptr);

  if (sbrk(0) != brk_val) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int brkMallocZero() {
  void* brk_val = sbrk(0);
  void* ptr = malloc(0);
  if (!ptr) {
    return MALLOC_FAILED;
  }

  void* current_brk = sbrk(0);
  if (current_brk == (void*) -1) {
    return BRK_VAL_FAILED;
  }

  if (brk_val == current_brk) {
    return BRK_VAL_FAILED;
  }

  free(ptr);

  if (sbrk(0) != brk_val) {
    return BRK_VAL_FAILED;
  }

  return SUCCESS;
}

int testPthreadBlockUsage() {
  pthread_t thread;

  // Thread erstellt, der eine kleine Allokation macht
  auto threadFunc = [](void*) -> void* {
    int* ptr = (int*) malloc(sizeof(int));
    if (!ptr) return (void*) MALLOC_FAILED;
    *ptr = 42; // Daten schreiben
    free(ptr); // Freigeben
    return nullptr;
  };

  if (pthread_create(&thread, nullptr, threadFunc, nullptr) != 0) {
    return THREAD_MALLOC_FAILED;
  }

  pthread_join(thread, nullptr);

  // Prüfen, ob die Heap-Liste sauber ist
  Block* heap = snp::Memory::getHeapList();
  if (heap) {
    return THREAD_FREE_FAILED;
  }

  return SUCCESS;
}

int testPthreadBrk() {
  pthread_t thread;

  auto threadFunc = [](void*) -> void* {
    int* ptr = (int*) malloc(100); // Eine kleine Allokation
    free(ptr);
    return nullptr;
  };

  void* initial_brk = sbrk(0);

  if (pthread_create(&thread, nullptr, threadFunc, nullptr) != 0) {
    return THREAD_MALLOC_FAILED;
  }

  pthread_join(thread, nullptr);

  void* final_brk = sbrk(0);

  if (initial_brk != final_brk) {
    return BRK_VAL_FAILED; // `brk` sollte auf den ursprünglichen Wert zurückgehen
  }

  return SUCCESS;
}

int testThreadSpecificAllocations() {
  pthread_t threads[TOTAL_THREAD_NUM];

  auto threadFunc = [](void*) -> void* {
    for (int i = 0; i < 10; ++i) {
      void* ptr = malloc(100); // Allokation
      free(ptr); // Freigabe
    }
    return nullptr;
  };

  for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
    if (pthread_create(&threads[i], nullptr, threadFunc, nullptr) != 0) {
      return THREAD_MALLOC_FAILED;
    }
  }

  for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
    pthread_join(threads[i], nullptr);
  }

  Block* heap = snp::Memory::getHeapList();
  if (heap) {
    return THREAD_FREE_FAILED; // Es sollten keine Blöcke mehr übrig sein
  }

  return SUCCESS;
}

int trimTest() {
  auto* brk_val = sbrk(0);
  Block* heap = snp::Memory::getHeapList();
  void* ptr1 = malloc(10);
  heap = snp::Memory::getHeapList();
  void* ptr2 = malloc(10);
  heap = snp::Memory::getHeapList();
  void* ptr3 = malloc(10);
  heap = snp::Memory::getHeapList();

  free(ptr1);
  heap = snp::Memory::getHeapList();
  free(ptr2);
  heap = snp::Memory::getHeapList();

  void* ptr4 = malloc(10);
  heap = snp::Memory::getHeapList();

  free(ptr3);
  free(ptr4);

  auto* brk_val2 = sbrk(0);

  if (brk_val != brk_val2) {
    return BRK_VAL_TRIM_FAILED;
  }

  return SUCCESS;
}

int test() {
  void* ptr = malloc(4016);
  void* ptr2 = malloc(470);
  Block* heap = snp::Memory::getHeapList();

  free(ptr);
  free(ptr2);

  return SUCCESS;
}

int main() {
  Block* heap_ = snp::Memory::getHeapList();
  snp::Memory::free(heap_ + 1);
  snp::Memory::setTotalAllocMemory(0);

  // Be creative ;)

  // Check all the requirements from our assignment description:
  // https://www.iaik.tugraz.at/teaching/materials/slp/assignments/a5/
  // Try to test as many as you can!
  // If you don't know how to test for something, you can ask for help
  // via discord or mail.

  // You can add new files in /tests/, c and c++ both work.

  assert(snp::Memory::total_alloc_memory() == 0);

  auto brk_val = sbrk(0);

  retCode(trimTest());
  auto current_brk = sbrk(0);
  retCode(mallocALot());
  current_brk = sbrk(0);
  retCode(checkMerging());
  current_brk = sbrk(0);
  retCode(smallMalloc());
  current_brk = sbrk(0);
  retCode(testBreak());
  current_brk = sbrk(0);
  retCode(nulllPtr());
  current_brk = sbrk(0);
  retCode(testLargeAllocation());
  current_brk = sbrk(0);
  retCode(testFragmentation());
  current_brk = sbrk(0);
  retCode(testCoalescing());
  current_brk = sbrk(0);
  retCode(testTrimLastBlock());
  current_brk = sbrk(0);
  retCode(testRandomAllocations());
  current_brk = sbrk(0);
  retCode(testMallocOneByte());
  current_brk = sbrk(0);
  retCode(testSmallAllocation());
  current_brk = sbrk(0);
  retCode(testSmallFree());
  current_brk = sbrk(0);
  retCode(ascendingMalloc());
  current_brk = sbrk(0);
  retCode(descendingMalloc());
  current_brk = sbrk(0);
  retCode(mallocZeroInbetween());
  current_brk = sbrk(0);

  retCode(noChangeBrk());
  current_brk = sbrk(0);
  retCode(brkExtension());
  current_brk = sbrk(0);
  retCode(brkReduction());
  current_brk = sbrk(0);
  retCode(fragBrk());
  current_brk = sbrk(0);
  retCode(testBrkMultipleExtensions());
  current_brk = sbrk(0);
  retCode(rlyBigAllocBrk());
  current_brk = sbrk(0);
  retCode(brkMallocZero());
  current_brk = sbrk(0);
  retCode(testBrk2());
  current_brk = sbrk(0);
  retCode(test());

  //retCode(testThreadSafety());
  //retCode(testPthreadBlockUsage());
  //retCode(testPthreadBrk());
  //retCode(testThreadSpecificAllocations());

  auto brk_val2 = sbrk(0);

  if (brk_val != brk_val2) {
    return TOTAL_BRK_VAL_FAILED;
  }

  return SUCCESS;
}
