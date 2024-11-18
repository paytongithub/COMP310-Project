#include "simple_malloc.h"

int main() {
    void* ptr1 = simple_malloc(12);
    void* ptr2 = simple_malloc(15);
    void* ptr3 = simple_malloc(25);
    simple_free(ptr1);
    simple_free(ptr2);
    void* ptr4 = simple_malloc(94);
    void* ptr5 = simple_malloc(1);
}
