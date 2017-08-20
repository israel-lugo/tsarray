#include <stdio.h>

#include "tsarray.h"

TSARRAY_TYPEDEF(intarray, int);


intarray a1 = TSARRAY_INITIALIZER;


int main(void)
{
    int count = 5;
    int i;

    printf("Adding %d ints starting from 50\n", count);
    for (i = 50; i < 50 + count; i++)
    {
        int res = intarray_append(&a1, &i);

        printf("append result: %d\n", res);
    }

    puts("\nReading all used items in array");

    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\nDeleting 3rd item");
    intarray_remove(&a1, 2);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_append(&a1, &i);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    return 0;
}
