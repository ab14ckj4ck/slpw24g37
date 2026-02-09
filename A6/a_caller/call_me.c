#include <stdio.h>
#include <stdint.h>
// There is nothing to do in this file.
// Changes in this file will not be considered in the test system.

#define MAX_SIZE 100

// This function has to be called.
/**
 * @brief Calculates the smallest moving sum given start, end and k of a double array.
 *
 * This function takes a pointer to a double array, a start index, an end index and the parameter k.
 * It calculates the smallest moving sum of elements within the specified range in the array and returns it.
 *
 * @param array            Pointer to the double array where the sum will be calculated.
 * @param start            The starting index of the range.
 * @param end              The ending index of the range.
 * @param k                The range of the moving sum.
 *
 * @return      The minimum moving sum of the specified range or
 *              -1 if the input is invalid.
 */
double calculateMinMovingSum(double* array, int start, int end, int k)
{
    printf("Function called with parameters: array=%p, start=%d, end=%d, k=%d\n",
           array, start, end, k);

    if (array == NULL || start < 0 || end < 0 || start > end || end > MAX_SIZE)
        return -1;

    double min = 9999;
    for (int i = start; i < end; i++)
    {
        double current = 0;
        for (int j = -k/2; j <= k/2; j++)
        {
            int index = i + j;
            if (index < 0 || index > MAX_SIZE)
                continue;
            current += array[index];
        }
        if (current < min)
            min = current;
    }
    return min;
}
