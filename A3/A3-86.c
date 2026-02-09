//
// Created by florian on 28.10.24.
//
#include <stdio.h>

int main() {
    const int num = 10;
    printf("Original value of num: %d\n", num);

    // num = 20; delete comment to see compiler error. Const can't be runtime modified.

    printf("Modified value of num: %d\n", num);
    return 0;
}
