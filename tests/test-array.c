#include <stdio.h>

#include <tsarray.h>


/*
 * Copied here from src/tsarray.c. This is internal use only. Must keep in
 * sync with the definition in src/tsarray.c.
 */
struct _tsarray_priv {
    struct _tsarray_pub pub;
    size_t obj_size;
    size_t capacity;
    size_t len;
};



TSARRAY_TYPEDEF(intarray, int);


intarray *a1;
intarray *a2;


static void _print_array(const intarray *a, const char *name)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a;
    int i;

    for (i=0; i < priv->len; i++)
        printf("%s[%d] = %d\n", name, i, a->items[i]);
}

static void _print_array_stats(const intarray *a, const char *name)
{
    struct _tsarray_priv *priv = (struct _tsarray_priv *)a;
    printf("%s- len: %lu, capacity: %lu\n", name, priv->len, priv->capacity);
}

#define print_array(a) _print_array(a, #a)
#define print_array_stats(a) _print_array_stats(a, #a)


int main(void)
{
    int count = 15;
    int i;
    int result;

    puts("a1- Creating new intarray");
    a1 = intarray_new();
    puts("a2- Creating new intarray");
    a2 = intarray_new();

    printf("a1- Adding %d ints starting from 50\n", count);
    print_array_stats(a1);
    for (i = 50; i < 50 + count; i++)
    {
        result = intarray_append(a1, &i);

        printf("append result: %d\n", result);
        print_array_stats(a1);
    }

    puts("\na1- Reading all items in array");

    print_array(a1);

    puts("\na1- Deleting 3rd item");
    result = intarray_remove(a1, 2);
    printf("remove result: %d\n", result);

    puts("\na1- Reading all items in array");
    print_array(a1);

    puts("\na1- Adding 69 to the array");
    i = 69;
    result = intarray_append(a1, &i);
    printf("append result: %d\n", result);

    puts("\na1- Reading all used items in array");
    print_array(a1);

    puts("\na2- Extending from a1");
    result = intarray_extend(a2, a1);
    printf("extend result: %d\n", result);
    print_array_stats(a2);

    puts("\na2- Reading all items in array");
    print_array(a2);

    puts("\na1- Cleaning up array");
    intarray_free(a1);
    a1 = intarray_new();
    print_array_stats(a1);

    puts("\na1- Appending 77 and 88");
    i = 77;
    result = intarray_append(a1, &i);
    printf("append result: %d\n", result);
    print_array_stats(a1);
    i = 88;
    result = intarray_append(a1, &i);
    printf("append result: %d\n", result);
    print_array_stats(a1);

    puts("\na1- Extending from itself, twice");
    result = intarray_extend(a1, a1);
    printf("extend result: %d\n", result);
    print_array_stats(a1);
    result = intarray_extend(a1, a1);
    printf("extend result: %d\n", result);
    print_array_stats(a1);

    puts("\na1- Reading all items in array");
    print_array(a1);

    puts("\na2- Extending from a1");
    result = intarray_extend(a2, a1);
    printf("extend result: %d\n", result);
    print_array_stats(a2);

    puts("\na2- Reading all items in array");
    print_array(a2);

    return 0;
}
