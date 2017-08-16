#include <stdio.h>

#include "tsarray.h"

TSARRAY_TYPEDEF(intarray, int);


intarray a1 = TSARRAY_INITIALIZER;


void print_array(void)
{
    int i;

    printf("\nArray len=%d, used_count=%d, min_len=%d. Used items:\n",
           a1.len, a1.used_count, a1.min_len);
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
    int retval;

    printf("Setting minimum array length 7\n");
    intarray_setminlen(&a1, 7);
    print_array();

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

    puts("\nTrying to truncate to len=2");
    retval = intarray_truncate(&a1, 2);
    printf("Return code: %d\n", retval);

    puts("\nSetting minimum length to 1");
    intarray_setminlen(&a1, 1);

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
