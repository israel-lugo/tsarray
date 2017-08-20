#include <stdio.h>

#include "tsarray.h"

TSARRAY_TYPEDEF(intarray, int);


intarray a1 = TSARRAY_INITIALIZER;


int main(void)
{
    int count = 15;
    int i;
    int result;

    printf("Adding %d ints starting from 50\n", count);
    printf("len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    for (i = 50; i < 50 + count; i++)
    {
        result = intarray_append(&a1, &i);

        printf("append result: %d\n", result);
        printf("len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    }

    puts("\nReading all used items in array");

    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\nDeleting 3rd item");
    result = intarray_remove(&a1, 2);
    printf("remove result: %d\n", result);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\nAdding 69 to the array");
    i = 69;
    result = intarray_append(&a1, &i);
    printf("append result: %d\n", result);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    return 0;
}
