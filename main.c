#include "simple_malloc.h"

int main() {
    void* ptr1 = simple_malloc(12);
    void* ptr2 = simple_malloc(15);
    void* ptr3 = simple_malloc(25);
    simple_free(ptr1);
    void* ptr4 = simple_malloc(94);
    void* ptr5 = simple_malloc(15);
    simple_free(ptr3);
    void* ptr6 = simple_malloc(26);
    ptr2 = simple_realloc(ptr2, 17);
    return 0;
}
