#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define BLOCK_HEADER_SIZE sizeof(BlockHeader)

int fptr;
char a[100];

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
    if (size == 0) return NULL;

    // Ensure alignment to 8 bytes
    size = (size + 7) & ~7;

    // Find the first-fit free block
    struct BlockHeader* block = find_first_fit(size);
    
    if (block != NULL) {
        // Split the block if there's excess space
        if (block->size >= size + BLOCK_HEADER_SIZE + 8) { // Ensure room for a valid new block
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

        fptr = open("malloc_log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        sprintf(a, "Allocated %zu bytes at address %p\n", size, (char*)block + BLOCK_HEADER_SIZE);
        write(fptr, a, strlen(a));
        close(fptr);

        analyze_malloc();
        return (char*)block + BLOCK_HEADER_SIZE;
    }

    // No suitable free block found, extend the heap
    block = sbrk(size + BLOCK_HEADER_SIZE);
    if (block == (void*)-1) {
        perror("sbrk failed");
        return NULL;
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;

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

    fptr = open("malloc_log.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    sprintf(a, "Extended heap and allocated %zu bytes at address %p\n", size, (char*)block + BLOCK_HEADER_SIZE);
    write(fptr, a, strlen(a));
    close(fptr);

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
    
    fptr = open("malloc_log.txt", O_WRONLY | O_APPEND);
    sprintf(a, "Freed %zu bytes at address %p\n", block->size, ptr);
    write(fptr, a, strlen(a));
    close(fptr);
// Attempt to coalesce with adjacent blocks
    coalesce(block);
    analyze_malloc();
}

void analyze_malloc() {
    struct BlockHeader* cursor = freeList;
    double totalSizeFree = 0;
    double largestFree = 0;
    while (cursor->next != NULL) {
	if (cursor->free != 0) {
	    if (cursor->size > largestFree) {
                largestFree = cursor->size;
	    }
	    totalSizeFree += (double)cursor->size;
	}
	cursor = cursor->next;
    }
    if (totalSizeFree <= 0) {
        fptr = open("malloc_log.txt", O_WRONLY | O_APPEND);
        sprintf(a, "No free blocks in the heap\n");
	write(fptr, a, strlen(a));
        close(fptr);
	return;
    }
    double frag = 0;
    if (totalSizeFree > 0) {
        frag = (totalSizeFree - largestFree) / totalSizeFree;
    }
    fptr = open("malloc_log.txt", O_WRONLY | O_APPEND);
    sprintf(a, "totalSizeFree: %f, largestFree: %f, Fragmentation: %f\n", totalSizeFree, largestFree, frag);
    write(fptr, a, strlen(a));
    close(fptr);
}


//ADD ANALYZE
void* simple_realloc(void* ptr, size_t new_size) {
    if (ptr == NULL) {
        // If the pointer is NULL, realloc behaves like malloc
        return simple_malloc(new_size);
    }

    if (new_size == 0) {
        // If the new size is 0, free the memory and return NULL
        simple_free(ptr);
        return NULL;
    }

    // Get the block header of the current block
    struct BlockHeader* block = (struct BlockHeader*)((char*)ptr - BLOCK_HEADER_SIZE);

    if (block->size >= new_size) {
        // If the current block is already large enough, no need to allocate a new block
        fptr = open("malloc_log.txt", O_WRONLY | O_APPEND);
        sprintf(a, "block->size >= new_size in realloc for %p\n", ptr);
	write(fptr, a, strlen(a));
	close(fptr);
        analyze_malloc(); //should this be different, maybe print a message that nothing changed to the log?
	return ptr;
    }

    // Check if we can expand into the adjacent free block
    if (block->next && block->next->free &&
        block->size + BLOCK_HEADER_SIZE + block->next->size >= new_size) {
        // Merge with the next block
        block->size += BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }

        if (block->size >= new_size) {	    
            fptr = open("malloc_log.txt", O_WRONLY | O_APPEND);
            sprintf(a, "expanded %p into the next block\n", ptr);
	    write(fptr, a, strlen(a));
	    close(fptr);
            analyze_malloc(); //add some kind of print to the log to explain?
            return ptr;
        }
    }

    // Allocate a new block
    void* new_ptr = simple_malloc(new_size);
    if (new_ptr == NULL) {
	//add something to log this?
        return NULL; // Allocation failed
    }

    // Copy data from the old block to the new block
    size_t copy_size = block->size < new_size ? block->size : new_size;
    memcpy(new_ptr, ptr, copy_size);

    // Free the old block
    simple_free(ptr);

    return new_ptr;
}

