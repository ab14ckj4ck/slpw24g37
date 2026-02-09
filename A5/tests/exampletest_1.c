#include "stdlib.h"
#include "string.h"

#ifdef assert
#undef assert
#endif
#define assert(val) do {\
 if ((val) == 0) exit(-1);\
} while (0);


// we declare the util functions here so the compiler knows about them
extern size_t total_alloc_memory();
extern size_t block_info(int type);



#define ARRAY_SIZE0 5
#define ARRAY_SIZE1 20
#define ARRAY_SIZE2 40
#define ARRAY_SIZE3 100
#define ARRAY_SIZE4 500
#define TOTAL_ARRAY_SIZE (ARRAY_SIZE0 + ARRAY_SIZE1 + ARRAY_SIZE2 + ARRAY_SIZE3 + ARRAY_SIZE4)

#define TEST_STRING "A string for testing."

#define SOME_CONSTANT 'A'
#define ANOTHER_CONSTANT 'X'

int main() {
  char* data[5];
  int i = 0;
  size_t info;
  assert(total_alloc_memory() == 0);

  data[0] = (char*) malloc(ARRAY_SIZE0 * sizeof(char));
  assert(data[0] != 0);
  assert(total_alloc_memory() == ARRAY_SIZE0 * sizeof(char));

  data[1] = (char*) malloc(ARRAY_SIZE1 * sizeof(char));
  data[2] = (char*) malloc(ARRAY_SIZE2 * sizeof(char));
  data[3] = (char*) malloc(ARRAY_SIZE3 * sizeof(char));
  data[4] = (char*) malloc(ARRAY_SIZE4 * sizeof(char));

  for (i = 0; i < ARRAY_SIZE2; ++i)
  {
    data[2][i] = SOME_CONSTANT;
  }

  strcpy(data[4], TEST_STRING);

  assert(total_alloc_memory() == (ARRAY_SIZE0 + ARRAY_SIZE1 + ARRAY_SIZE2 + ARRAY_SIZE3 + ARRAY_SIZE4) * sizeof(char));

  free(data[0]);
  free(data[1]);
  free(data[3]);

  info = block_info(1);
  assert(info == ARRAY_SIZE2 * sizeof(char));

  for (i = 0; i < ARRAY_SIZE2; ++i)
  {
    if (data[2][i] != SOME_CONSTANT)
    {
      assert(0);
    }
  }

  if (strcmp(data[4], TEST_STRING))
  {
    assert(0);
  }
  
  data[0] = (char*) malloc(25 * sizeof(char));

  assert(total_alloc_memory() == (ARRAY_SIZE0 + ARRAY_SIZE1 + ARRAY_SIZE2 + ARRAY_SIZE3 + ARRAY_SIZE4 + 25) * sizeof(char));

  free(data[0]);
  free(data[2]);
  free(data[4]);

  return 0;
}
