#include <unistd.h>
#include <stddef.h>
#include <stdio.h>

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

FILE *fptr;

// Block header structure to track each allocation
struct BlockHeader {
    size_t size;               // Size of the block
    int free;                  // Whether the block is free or not
    struct BlockHeader* next;  // Pointer to the next block
    struct BlockHeader* prev;  // Pointer to the previous block
} BlockHeader;

static struct BlockHeader* freeList = NULL;  // Start of the free list

// Utility function to find the first-fit free block
struct BlockHeader* find_first_fit(size_t size) {
    struct BlockHeader* current = freeList;
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
    struct BlockHeader* block = find_first_fit(size);

    if (block != NULL) {
        // Split the block if there's excess space
        if (block->size >= size + BLOCK_HEADER_SIZE) {
            struct BlockHeader* newBlock = (struct BlockHeader*)((char*)block + BLOCK_HEADER_SIZE + size);
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

        fptr = fopen("malloc_log.txt", "a");
        fprintf(fptr, "Allocated %zu bytes at address %p (Block total size: %zu)\n", size, (char*)block + BLOCK_HEADER_SIZE, block->size + BLOCK_HEADER_SIZE);
	fclose(fptr);
	analyze_malloc();
	// TODO: Write code to print current heap structur
	// Print allocated bytes, then visualization of heap OR metric of fragmentation.
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
        struct BlockHeader* last = freeList;
        while (last->next) {
            last = last->next;
        }
        last->next = block;
        block->prev = last;
    }

    fptr = fopen("malloc_log.txt", "a");
    fprintf(fptr, "Extended heap and allocated %zu bytes at address %p (Block total size: %zu)\n", size, (char*)block + BLOCK_HEADER_SIZE, block->size + BLOCK_HEADER_SIZE);
    fclose(fptr);
    analyze_malloc();
    return (char*)block + BLOCK_HEADER_SIZE;
}

void coalesce(struct BlockHeader* block) {
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

    struct BlockHeader* block = (struct BlockHeader*)((char*)ptr - BLOCK_HEADER_SIZE);
    block->free = 1;
    
    fptr = fopen("malloc_log.txt", "a");
    fprintf(fptr, "Freed %zu bytes at address %p\n", block->size, ptr);
    fclose(fptr);
// Attempt to coalesce with adjacent blocks
    coalesce(block);
    analyze_malloc();
}

void analyze_malloc() {
    struct BlockHeader* cursor = freeList;
    int totalSize = 0;
    int count = 0;
    while (cursor->next != NULL) {
	if (cursor->free != 0) {
	    count++;
	    totalSize += (int)cursor->size;
	}
	cursor = cursor->next;
    }
    int average = 0;
    if (count > 0) {
	average = totalSize / count;
    }
    fptr = fopen("malloc_log.txt", "a");
    fprintf(fptr, "Avergae size of free block: %i\n", average);
    fclose(fptr);
}
