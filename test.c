#include <stdio.h>

#include "tsarray.h"

TSARRAY_TYPEDEF(intarray, int);


intarray a1 = TSARRAY_EMPTY;


int main(void)
{
    int count = 5;
    int i;

    printf("Adding %d ints starting from 50\n", count);
    for (i = 50; i < 50 + count; i++)
    {
        int idx = intarray_add(&a1, &i);

        printf("added idx: %d\n", idx);
    }

    puts("\nReading all used items in array");

    for (i = 0; i < a1.len; i++)
    {
        if (a1.items[i].used)
            printf("a1[%d] = %d\n", i, a1.items[i].object);
    }

    puts("\nDeleting 3rd item");
    intarray_remove(&a1, 2);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        if (a1.items[i].used)
            printf("a1[%d] = %d\n", i, a1.items[i].object);
    }

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_add(&a1, &i);

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        if (a1.items[i].used)
            printf("a1[%d] = %d\n", i, a1.items[i].object);
    }

    return 0;
}
