#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdbool.h>

// Metadata structure for each allocated block


// Function prototypes
void allocator_init(void *base);
void allocator_reset();
void *mymalloc(size_t size);
void myfree(void *ptr);
void *myrealloc(void *ptr, size_t size);

// Helper function prototypes
// block_metadata_t *get_metadata(void *ptr);
// void *get_user_ptr(block_metadata_t *metadata);
// block_metadata_t *find_last_block();
// void add_to_free_list(block_metadata_t *block);
// void remove_from_free_list(block_metadata_t *block);

#endif // ALLOCATOR_H
