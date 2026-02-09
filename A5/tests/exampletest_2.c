#include "time.h"
#include <stdlib.h>

#ifdef assert
#undef assert
#endif
#define assert(val) do {\
 if ((val) == 0) exit(-1);\
} while (0);


// we declare the util functions here so the compiler knows about them
extern size_t total_alloc_memory();
extern size_t block_info(int type);

#define MAX_ITER_COUNT 1000

int random_malloc_free_read_write_impl(int minsize, int maxsize,
                                        int numiter, int ratio){
  int *data[MAX_ITER_COUNT];
  size_t sizes[MAX_ITER_COUNT];
  int i, j, index;
  int aktion;
  size_t size;
  int d = maxsize - minsize;
  int errors = 0;
  int numallocs = 0;
  int numfree = 0;

  if (numiter > MAX_ITER_COUNT)
  {
      return -1;
  }
  for (i = 0; i < MAX_ITER_COUNT; ++i)
  {
      data[i] = 0;
      sizes[i] = 0;
  }

  for (i = 0; i < numiter; ++i)
  {
    aktion = (int) random();
    aktion = aktion % 10;
    if (aktion >= ratio)
    {
      if (d > 0)
      {
        size = (size_t) random();
        size = size % ((size_t) d) + (size_t) minsize;
      }
      else
      {
        size = (size_t) minsize;
      }
      data[i] = (int*) malloc(size * sizeof(int));
      numallocs++;
      sizes[i] = size;

      for (j = 0; j < (int) size; ++j)
      {
        data[i][j] = i;
      }
    }
    else
    {
      if (numallocs <= numfree)
      {
        continue;
      }

      do
      {
        index = (int) random();
        index = index % (i + 1);
      } while (data[index] == 0);

      for (j = 0; j < (int) sizes[index]; ++j)
      {
        if (data[index][j] != index)
        {
          errors++;
        }
      }
      free(data[index]);
      ++numfree;
      data[index] = 0;
    }
  }

  for(i = 0; i < numiter; ++i)
  {
    free(data[i]);
  }
  return (errors != 0);
}


int main(){
  int minsize = 200;
  int maxsize = 1000;
  int numiter = 99;
  int ratio = 3;
  int error_count = 0;
  int i;

  srand((unsigned int) time(NULL));

  for (i = 0; i < 15; ++i){
    // Grab random values.
    minsize = rand();
    minsize = minsize % 900 + 100;
    maxsize = rand();
    maxsize = maxsize % 1500 + 200;
    numiter = rand();
    numiter = numiter % 90 + 10;
    ratio = rand();
    ratio = ratio % 10;

    // Perform check with those values, add up errors
    error_count += random_malloc_free_read_write_impl(minsize, maxsize, numiter, ratio);
  }
  if (error_count == 0)
  {
    return 0;
  }
  return 1;
}
