#include "stdlib.h"


int main()
{
  // Be creative ;)

  // Check all the requirements from our assignment description:
  // https://www.iaik.tugraz.at/teaching/materials/slp/assignments/a5/
  // Try to test as many as you can!
  // If you don't know how to test for something, you can ask for help
  // via discord or mail.

  // You can add new files in /tests/, c and c++ both work.
  void* ptr = malloc(1024 * 100);
  if (ptr == NULL) {
    return -1;
  }

  free(ptr);


  char* ptr_c = (char*)calloc(10, sizeof(char));
  if (ptr_c == NULL) {
    return -9;
  }

  for (int i = 0; i < 10; i++) {
    if (ptr_c[i] != 0) {
      free(ptr_c);
      return -2;
    }
  }

  free(ptr_c);

  ptr_c = (char*)malloc(10 * sizeof(char));

  for (int i = 0; i < 11; i++) {
    ptr_c[i] = 1;
  }

  free(ptr_c);


  return 0;
}
