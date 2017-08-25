#include <stdio.h>

#include "tsarray.h"

TSARRAY_TYPEDEF(intarray, int);


intarray a1 = TSARRAY_INITIALIZER;
intarray a2 = TSARRAY_INITIALIZER;


int main(void)
{
    int count = 15;
    int i;
    int result;

    printf("a1- Adding %d ints starting from 50\n", count);
    printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    for (i = 50; i < 50 + count; i++)
    {
        result = intarray_append(&a1, &i);

        printf("append result: %d\n", result);
        printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    }

    puts("\na1- Reading all items in array");

    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\na1- Deleting 3rd item");
    result = intarray_remove(&a1, 2);
    printf("remove result: %d\n", result);

    puts("\na1- Reading all items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\na1- Adding 69 to the array");
    i = 69;
    result = intarray_append(&a1, &i);
    printf("append result: %d\n", result);

    puts("\na1- Reading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\na2- Extending from a1");
    result = intarray_extend(&a2, &a1);
    printf("extend result: %d\n", result);
    printf("a2- len: %lu, capacity: %lu\n", a2.len, a2._priv.capacity);

    puts("\na2- Reading all items in array");
    for (i = 0; i < a2.len; i++)
    {
        printf("a2[%d] = %d\n", i, a2.items[i]);
    }

    puts("\na1- Cleaning up array");
    intarray_free(&a1);
    printf("len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);

    puts("\na1- Appending 77 and 88");
    i = 77;
    result = intarray_append(&a1, &i);
    printf("append result: %d\n", result);
    printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    i = 88;
    result = intarray_append(&a1, &i);
    printf("append result: %d\n", result);
    printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);

    puts("\na1- Extending from itself, twice");
    result = intarray_extend(&a1, &a1);
    printf("extend result: %d\n", result);
    printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);
    result = intarray_extend(&a1, &a1);
    printf("extend result: %d\n", result);
    printf("a1- len: %lu, capacity: %lu\n", a1.len, a1._priv.capacity);

    puts("\na1- Reading all items in array");
    for (i = 0; i < a1.len; i++)
    {
        printf("a1[%d] = %d\n", i, a1.items[i]);
    }

    puts("\na2- Extending from a1");
    result = intarray_extend(&a2, &a1);
    printf("extend result: %d\n", result);
    printf("a2- len: %lu, capacity: %lu\n", a2.len, a2._priv.capacity);

    puts("\na2- Reading all items in array");
    for (i = 0; i < a2.len; i++)
    {
        printf("a2[%d] = %d\n", i, a2.items[i]);
    }

    return 0;
}
