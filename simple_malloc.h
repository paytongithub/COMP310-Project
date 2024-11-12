#ifndef SIMPLE_MALLOC_H
#define SIMPLE_MALLOC_H

typedef struct BlockHeader {
    size_t size;               // Size of the block
    int free;                  // Whether the block is free or not
    struct BlockHeader* next;  // Pointer to the next block
    struct BlockHeader* prev;  // Pointer to the previous block
};

BlockHeader* find_first_fit(size_t size);
void* simple_malloc(size_t size);
void coalesce(BlockHeader* block);
void simple_free(void* ptr);

#endif
