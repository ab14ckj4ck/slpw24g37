# SLP A5

All you need to know about this assignment can be found in the assignment description linked below:
[https://www.iaik.tugraz.at/teaching/materials/slp/assignments/a5/](https://www.iaik.tugraz.at/teaching/materials/slp/assignments/a5/)
### Compile and Run your program
- To compile everything, type `make` in your repository.
- You will find the binaries for your tests in the `/build/` folder, you can start them using `./build/testname`
- You can add and push as many new files in the `/tests/` folder as you like.
- If a test starts with "FAIL_", the run script will check to make sure the test does in fact fail. (see `tests/FAIL_basic.c`)
  - If you expect a specific returnvalue, for example `139` for a segfault, you can add the number into the testname
  - The testrunner will then alert you, if the test failed with the wrong returnvalue. (For example: a test that should segfault times out)
- You can modify the `Makefile` and `util.cpp` to your liking, but those files will be disregarded on the testsystem.
  - We strongly encourage you to write your own util functions and include them in the util.cpp for your own testing


