#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define SIZE 95

// Implement the given function (see the wiki) in the sysv_abi.S file
uint64_t calculate_fibonacci(uint64_t* fibonacci_numbers, uint64_t amount);

int main(int argc, char **argv)
{
    setbuf(stdout, NULL); // Disable buffering

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <amount of fibonacci numbers>\n", argv[0]);
        return 1;
    }

    uint64_t fibonacci_numbers[SIZE] = {0};
    uint64_t amount = strtoull(argv[1], 0L, 10);

    if (amount >= SIZE) 
    {
        fprintf(stderr, "Error: amount needs to be smaller that 95");
        return 1;
    }

    uint64_t retval = calculate_fibonacci(fibonacci_numbers, amount);
    printf("Returned Fibonacci number: %lu\n", retval);

    return 0;
}
