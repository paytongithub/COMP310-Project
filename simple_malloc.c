#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

// Block header structure to track each allocation
typedef struct BlockHeader {
    size_t size;               // Size of the block
    int free;                  // Whether the block is free or not
    struct BlockHeader* next;  // Pointer to the next block
    struct BlockHeader* prev;  // Pointer to the previous block
} BlockHeader;

static BlockHeader* freeList = NULL;  // Start of the free list

// Utility function to find the first-fit free block
BlockHeader* find_first_fit(size_t size) {
    BlockHeader* current = freeList;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void* simple_malloc(size_t size) {
    // Find the first-fit free block
    BlockHeader* block = find_first_fit(size);

    if (block != NULL) {
        // Split the block if there's excess space
        if (block->size >= size + BLOCK_HEADER_SIZE) {
            BlockHeader* newBlock = (BlockHeader*)((char*)block + BLOCK_HEADER_SIZE + size);
            newBlock->size = block->size - size - BLOCK_HEADER_SIZE;
            newBlock->free = 1;
            newBlock->next = block->next;
            newBlock->prev = block;

            if (newBlock->next) {
                newBlock->next->prev = newBlock;
            }

            block->size = size;
            block->next = newBlock;
        }

        block->free = 0;
        return (char*)block + BLOCK_HEADER_SIZE;
    }

    // No suitable free block found, extend the heap
    block = sbrk(size + BLOCK_HEADER_SIZE);
    if (block == (void*)-1) {
        return NULL;  // sbrk failed
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;

    // Attach to the end of the free list
    if (freeList == NULL) {
        freeList = block;
    } else {
        BlockHeader* last = freeList;
        while (last->next) {
            last = last->next;
        }
        last->next = block;
        block->prev = last;
    }

    return (char*)block + BLOCK_HEADER_SIZE;
}

void coalesce(BlockHeader* block) {
    // Merge with next block if it's free
    if (block->next && block->next->free) {
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Merge with previous block if it's free
    if (block->prev && block->prev->free) {
        block->prev->size += BLOCK_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

void simple_free(void* ptr) {
    if (ptr == NULL) return;

    BlockHeader* block = (BlockHeader*)((char*)ptr - BLOCK_HEADER_SIZE);
    block->free = 1;

    // Attempt to coalesce with adjacent blocks
    coalesce(block);
}

int main() {
    // Example usage
    void* ptr1 = simple_malloc(100);
    printf("Allocated 100 bytes at %p\n", ptr1);

    void* ptr2 = simple_malloc(200);
    printf("Allocated 200 bytes at %p\n", ptr2);

    simple_free(ptr1);
    printf("Freed 100 bytes\n");

    void* ptr3 = simple_malloc(50);
    printf("Allocated 50 bytes at %p\n", ptr3);

    simple_free(ptr2);
    simple_free(ptr3);
    printf("Freed remaining allocations\n");

    return 0;
}
