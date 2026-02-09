#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
// Implement the function call in assembly in the TODO section below.
// Do not change the functionality or the signature of the function to be called.
// Do not modify this file outside the TODO section.

#define MAX_SIZE 100

// Function to be called:
double calculateMinMovingSum(double* array, int start, int end, int k);

int main(int argc, char **argv)
{
    setbuf(stdout, NULL); // Disable buffering

    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <file> <start index> <end index> <k>\n", argv[0]);
        fprintf(stderr, "For example: %s test.txt 1 9 3\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }

    int start = atoi(argv[2]);
    int end = atoi(argv[3]);
    int k = atoi(argv[4]);

    double array[MAX_SIZE] = {0};
    int count = 0;

    while (count < MAX_SIZE && fscanf(file, "%lf", &array[count]) == 1)
        count++;

    fclose(file);

    double result;

    printf("Calling function with parameters: array=%p, start=%d, end=%d, k=%d\n",
           array, start, end, k);

    // Use (only!) GCC inline assembly to call `calculateMinMovingSum`. Always consider the expected data types.
    // Parameters: array, start, end, target_sum, &sub_array_sum
    // Store the return value in result
    // You may want to use movq as there are no proper constraints for doubles!

    // ########## TODO START ##########
    // System V AMD64 ABI implementation (64-bit)


    // ########## TODO END ##########

    printf("Function returned: %.2lf\n", result);
    printf("The minimum moving sum from %d to %d is %.2lf\n", start, end, result);

    return 0;
}
