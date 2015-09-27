#include <stdio.h>

#include "dynarray.h"

DYNARRAY_TYPE_DECLARE(intarray, int);


intarray a1 = DYNARRAY_EMPTY;


void print_array(void)
{
    int i;

    puts("\nReading all used elements in array");
    for (i = 0; i < a1.len; i++)
    {
        if (a1.elements[i].used)
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

    puts("\nDeleting 3rd element");
    intarray_remove(&a1, 2);

    print_array();

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_add(&a1, &i);

    print_array();

    puts("\nDeleting 3rd element again");
    intarray_remove(&a1, 2);

    print_array();

    puts("\nCompacting array");
    intarray_compact(&a1, 0);

    print_array();

    puts("\nAdding 69 to the array");
    i = 69;
    intarray_add(&a1, &i);

    print_array();

    return 0;
}
