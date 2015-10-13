#include <stdio.h>

#include "dynarray.h"

DYNARRAY_TYPE_DECLARE(intarray, int);


intarray a1 = DYNARRAY_EMPTY;


void print_array(void)
{
    int i;

    puts("\nReading all used items in array");
    for (i = 0; i < a1.len; i++)
    {
        if (a1.items[i].used)
            printf("a1[%d] = %d\n", i, *intarray_get_nth(&a1, i));
    }
}


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

    print_array();

    puts("\nDeleting 3rd item");
    intarray_remove(&a1, 2);

    print_array();

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_add(&a1, &i);

    print_array();

    puts("\nDeleting 3rd item again");
    intarray_remove(&a1, 2);

    print_array();

    puts("\nDeleting 4th item");
    intarray_remove(&a1, 3);

    print_array();

    puts("\nCompacting array");
    intarray_compact(&a1, 0);

    print_array();

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_add(&a1, &i);

    print_array();

    puts("\nTruncating the array to len=2");
    intarray_truncate(&a1, 2);

    print_array();

    puts("\nAdding 3rd and 4th items");
    i = 73;
    intarray_add(&a1, &i);
    i = 74;
    intarray_add(&a1, &i);

    print_array();

    puts("\nRemoving 3rd item");
    intarray_remove(&a1, 2);

    print_array();

    puts("\nTruncating the array to len=2");
    intarray_truncate(&a1, 2);

    print_array();

    return 0;
}
