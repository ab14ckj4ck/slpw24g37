#include <stddef.h>
#include <stdlib.h>
#include <new>

class Test {
  private:
  int i_;

  public:
  Test(int i) : i_(i) {}
  ~Test() {}

  int get() { return i_; }
  void set(int i) { i_ = i; }

};

int main()
{
  // Be sure to test your new-operator implementation ;)
  try {
  Test* t = new Test(42);

  if (t->get() != 42) return -1;

  t->set(23);

  if (t->get() != 23) return -1;

  delete t; }
  catch (std::bad_alloc& e) {
    return -1;
  }
  return 0;
}
