//
// Created by florian on 27.10.24.
//

//functions in code segment. init in data segment, no-init in bss segment. Init closer to code than bss.
#include <stdio.h>

int var;
int init_var = 10;

int init_var_2 = 20;
int var_2;

void myFunction() {
    printf("Hello World");
}

int main() {
    printf("Address of myFunction: %p\n", myFunction);
    printf("Address of var: %p\n", &var);
    printf("Address of init_var: %p\n", &init_var);
    printf("Address of init_var_2: %p\n", &init_var_2);
    printf("Address of var_2: %p\n", &var_2);
}