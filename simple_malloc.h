#ifndef SIMPLE_MALLOC_H
#define SIMPLE_MALLOC_H

#include <stddef.h>  // For size_t
#include <stdio.h>   // For FILE

// Block header structure to track each allocation
struct BlockHeader {
    size_t size;               // Size of the block
    int free;                  // Whether the block is free or not
    struct BlockHeader* next;  // Pointer to the next block
    struct BlockHeader* prev;  // Pointer to the previous block
};

#define BLOCK_HEADER_SIZE sizeof(struct BlockHeader)

extern FILE *fptr;  // Declare file pointer for logging

// Free list pointer
extern struct BlockHeader* freeList;

// Function declarations
struct BlockHeader* find_first_fit(size_t size);
void* simple_malloc(size_t size);
void coalesce(struct BlockHeader* block);
void simple_free(void* ptr);
void analyze_malloc();

#endif // SIMPLE_MALLOC_H

