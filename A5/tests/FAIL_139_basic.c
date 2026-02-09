#include "stdlib.h"


int main()
{
  int* data = malloc(100);
  
  data[100000] = 5;


  return 0;
}
