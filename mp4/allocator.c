// #include "allocator.h"
// #include <string.h> 
// #include <stdbool.h> 

// //new
// #define NUM_SIZE_CLASSES 16

// typedef struct block_metadata {
//     size_t size;       
//     bool is_free;      
//     struct block_metadata *next_free; 
//     struct block_metadata *prev_free;
//     int size_class; //new 
// } block_metadata_t;

// static void *base;             
// static size_t used;            
// static block_metadata_t *free_list_head = NULL; 
// static block_metadata_t *free_list_tail = NULL;
// //new
// static block_metadata_t *free_lists[NUM_SIZE_CLASSES] = {NULL};
// //new
// int get_size_class(size_t size) {
//   // Exponential size classes
//   // 16, 32, 64, 128, 256, 512, 1K, 2K, 4K, 8K, 16K, 32K, 64K, 128K, 256K, 512K+
//   int class = 0;
//   size_t class_size = 16;
//   while (class < NUM_SIZE_CLASSES - 1 && size > class_size) {
//       class++;
//       class_size *= 2;
//   }
//   return class;
// }
// //~~~~~~
// block_metadata_t *get_metadata(void *ptr) {
//     if (ptr == NULL) return NULL;
//     return (block_metadata_t *)((char *)ptr - sizeof(block_metadata_t));
// }

// void *get_user_ptr(block_metadata_t *metadata) {
//     if (metadata == NULL) return NULL;
//     return (char *)metadata + sizeof(block_metadata_t);
// }

// // void add_to_free_list(block_metadata_t *block) {
// //     if (!block) return;

// //     block->is_free = true;
// //     block->next_free = NULL;
// //     block->prev_free = NULL;
// //     if (!free_list_head) {
// //         free_list_head = block;
// //         free_list_tail = block;
// //         return;
// //     }

// //     block->prev_free = free_list_tail;
// //     free_list_tail->next_free = block;
// //     free_list_tail = block;
// // }

// void add_to_free_list(block_metadata_t *block) {
//   if (!block) return;

//   block->is_free = true;
//   block->size_class = get_size_class(block->size);
//   block->next_free = free_lists[block->size_class];
//   block->prev_free = NULL;

//   if (free_lists[block->size_class]) {
//       free_lists[block->size_class]->prev_free = block;
//   }
//   free_lists[block->size_class] = block;
// }

// // void remove_from_free_list(block_metadata_t *block) {
// //     if (!block || !free_list_head) return;

// //     if (block->prev_free) {
// //         block->prev_free->next_free = block->next_free;
// //     } else {
// //         free_list_head = block->next_free;
// //     }

// //     if (block->next_free) {
// //         block->next_free->prev_free = block->prev_free;
// //     } else {
// //         free_list_tail = block->prev_free;
// //     }

// //     block->prev_free = NULL;
// //     block->next_free = NULL;
// // }

// void remove_from_free_list(block_metadata_t *block) {
//   if (!block) return;

//   // Update next block's prev pointer
//   if (block->next_free) {
//       block->next_free->prev_free = block->prev_free;
//   }

//   // Update prev block's next pointer
//   if (block->prev_free) {
//       block->prev_free->next_free = block->next_free;
//   } else {
//       // If no prev block, this was the head of its size class list
//       free_lists[block->size_class] = block->next_free;
//   }

//   block->next_free = NULL;
//   block->prev_free = NULL;
// }

// block_metadata_t *find_block_after(block_metadata_t *block) {
//     char *block_end = (char *)block + sizeof(block_metadata_t) + block->size;
//     char *heap_end = (char *)base + used;
    
//     if (block_end >= heap_end) return NULL;
    
//     return (block_metadata_t *)block_end;
// }

// block_metadata_t *find_block_before(block_metadata_t *block) {
//     char *heap_start = (char *)base;
//     char *current = heap_start;
//     block_metadata_t *prev_block = NULL;
    
//     while (current < (char *)block) {
//         block_metadata_t *current_block = (block_metadata_t *)current;
//         if ((char *)current_block + sizeof(block_metadata_t) + current_block->size > (char *)block) {
//             return NULL;  // Overlapping blocks or block not found
//         }
        
//         prev_block = current_block;
//         current = (char *)current_block + sizeof(block_metadata_t) + current_block->size;
//     }
    
//     return prev_block;
// }

// void merge_adjacent_free_blocks(block_metadata_t *block) {
//     block_metadata_t *next_block = find_block_after(block);
//     block_metadata_t *prev_block = find_block_before(block);
    
//     if (next_block && next_block->is_free) {
//         remove_from_free_list(next_block);
        
//         block->size += sizeof(block_metadata_t) + next_block->size;
//     }
    
//     if (prev_block && prev_block->is_free) {
//         remove_from_free_list(block);        
//         prev_block->size += sizeof(block_metadata_t) + block->size;        
//         block = prev_block;
//     }
    
//     if (block->is_free) {
//         remove_from_free_list(block);
//         add_to_free_list(block);
//     }
// }

// // block_metadata_t *find_free_block(size_t size) {
// //     block_metadata_t *current = free_list_head;
    
// //     while (current) {
// //         if (current->is_free && current->size >= size) {
// //             return current;
// //         }
// //         current = current->next_free;
// //     }
    
// //     return NULL;
// // }

// block_metadata_t *find_free_block(size_t size) {
//   int start_class = get_size_class(size);

//   // Look for the smallest size class that can accommodate the request
//   for (int class = start_class; class < NUM_SIZE_CLASSES; class++) {
//       block_metadata_t *current = free_lists[class];
//       while (current) {
//           if (current->size >= size) {
//               return current;
//           }
//           current = current->next_free;
//       }
//   }
  
//   return NULL;
// }

// void split_block(block_metadata_t *block, size_t size) {
//     size_t min_split_size = sizeof(block_metadata_t) + 16;    
//     if (block->size >= size + min_split_size) {
//         size_t remainder_size = block->size - size - sizeof(block_metadata_t);
        
//         block_metadata_t *new_block = (block_metadata_t *)((char *)block + sizeof(block_metadata_t) + size);
//         new_block->size = remainder_size;
//         new_block->is_free = true;
        
//         block->size = size;
        
//         add_to_free_list(new_block);
//     }
// }

// void allocator_init(void *newbase) {
//     base = newbase;
//     used = 0;
//     free_list_head = NULL;
//     free_list_tail = NULL;
// }

// void allocator_reset() { 
//     used = 0; 
//     free_list_head = NULL;
//     free_list_tail = NULL;
// }

// void *mymalloc(size_t size) {
//     if (size == 0) return NULL;
    
//     block_metadata_t *reuse_block = find_free_block(size);
//     if (reuse_block) {
//         reuse_block->is_free = false;
        
//         remove_from_free_list(reuse_block);
        
//         split_block(reuse_block, size);
        
//         return get_user_ptr(reuse_block);
//     }
    
//     block_metadata_t *new_block = (block_metadata_t *)(((char *)base) + used);
//     new_block->size = size;
//     new_block->is_free = false;
//     new_block->next_free = NULL;
//     new_block->prev_free = NULL;
    
//     used += sizeof(block_metadata_t) + size;
    
//     return get_user_ptr(new_block);
// }

// void myfree(void *ptr) {
//     if (ptr == NULL) return;
    
//     block_metadata_t *block = get_metadata(ptr);
    
//     block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
//     if (block == last_block) {
//         used -= (sizeof(block_metadata_t) + block->size);
//         return;
//     }
    
//     add_to_free_list(block);
    
//     merge_adjacent_free_blocks(block);
// }

// void *myrealloc(void *ptr, size_t size) {
//     if (size == 0) { 
//         myfree(ptr); 
//         return NULL; 
//     }
    
//     if (ptr == NULL) {
//         return mymalloc(size);
//     }
    
//     block_metadata_t *block = get_metadata(ptr);
    
//     if (size <= block->size) {
//         block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
        
//         if (block == last_block) {
//             size_t size_diff = block->size - size;
//             block->size = size;
//             used -= size_diff;
//         } else {
//             split_block(block, size);
//         }
        
//         return ptr;
//     }
    
//     block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
//     if (block == last_block) {
//         size_t additional_space = size - block->size;
        
//         block->size = size;
//         used += additional_space;
        
//         return ptr;
//     }
    
//     block_metadata_t *next_block = find_block_after(block);
//     if (next_block && next_block->is_free && 
//         block->size + sizeof(block_metadata_t) + next_block->size >= size) {
//         remove_from_free_list(next_block);
        
//         block->size += sizeof(block_metadata_t) + next_block->size;
        
//         split_block(block, size);
        
//         return ptr;
//     }
    
//     void *new_ptr = mymalloc(size);
//     if (new_ptr) {
//         memcpy(new_ptr, ptr, block->size); 
//         myfree(ptr);
//     }
    
//     return new_ptr;
// }




//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "allocator.h"
#include <string.h> 
#include <stdbool.h> 

#define NUM_SIZE_CLASSES 16

typedef struct block_metadata {
    size_t size;       
    bool is_free;      
    struct block_metadata *next_free; 
    struct block_metadata *prev_free; 
    int size_class;    // New field to track size class
} block_metadata_t;

static void *base;             
static size_t used;            

// Array of free lists, organized by size classes
static block_metadata_t *free_lists[NUM_SIZE_CLASSES] = {NULL};

// Utility function to determine size class
int get_size_class(size_t size) {
    // Exponential size classes
    // 16, 32, 64, 128, 256, 512, 1K, 2K, 4K, 8K, 16K, 32K, 64K, 128K, 256K, 512K+
    int class = 0;
    size_t class_size = 16;
    while (class < NUM_SIZE_CLASSES - 1 && size > class_size) {
        class++;
        class_size *= 2;
    }
    return class;
}

block_metadata_t *get_metadata(void *ptr) {
    if (ptr == NULL) return NULL;
    return (block_metadata_t *)((char *)ptr - sizeof(block_metadata_t));
}

void *get_user_ptr(block_metadata_t *metadata) {
    if (metadata == NULL) return NULL;
    return (char *)metadata + sizeof(block_metadata_t);
}

block_metadata_t *find_block_after(block_metadata_t *block) {
    char *block_end = (char *)block + sizeof(block_metadata_t) + block->size;
    char *heap_end = (char *)base + used;
    
    if (block_end >= heap_end) return NULL;
    
    return (block_metadata_t *)block_end;
}

block_metadata_t *find_block_before(block_metadata_t *block) {
    char *heap_start = (char *)base;
    char *current = heap_start;
    block_metadata_t *prev_block = NULL;
    
    while (current < (char *)block) {
        block_metadata_t *current_block = (block_metadata_t *)current;
        if ((char *)current_block + sizeof(block_metadata_t) + current_block->size > (char *)block) {
            return NULL;  // Overlapping blocks or block not found
        }
        
        prev_block = current_block;
        current = (char *)current_block + sizeof(block_metadata_t) + current_block->size;
    }
    
    return prev_block;
}

void add_to_free_list(block_metadata_t *block) {
    if (!block) return;

    block->is_free = true;
    block->size_class = get_size_class(block->size);
    block->next_free = free_lists[block->size_class];
    block->prev_free = NULL;

    if (free_lists[block->size_class]) {
        free_lists[block->size_class]->prev_free = block;
    }
    free_lists[block->size_class] = block;
}

void remove_from_free_list(block_metadata_t *block) {
    if (!block) return;

    // Update next block's prev pointer
    if (block->next_free) {
        block->next_free->prev_free = block->prev_free;
    }

    // Update prev block's next pointer
    if (block->prev_free) {
        block->prev_free->next_free = block->next_free;
    } else {
        // If no prev block, this was the head of its size class list
        free_lists[block->size_class] = block->next_free;
    }

    block->next_free = NULL;
    block->prev_free = NULL;
}

void merge_adjacent_free_blocks(block_metadata_t *block) {
    block_metadata_t *next_block = find_block_after(block);
    block_metadata_t *prev_block = find_block_before(block);
    
    if (next_block && next_block->is_free) {
        remove_from_free_list(next_block);
        
        block->size += sizeof(block_metadata_t) + next_block->size;
    }
    
    if (prev_block && prev_block->is_free) {
        remove_from_free_list(block);        
        prev_block->size += sizeof(block_metadata_t) + block->size;        
        block = prev_block;
    }
    
    if (block->is_free) {
        remove_from_free_list(block);
        add_to_free_list(block);
    }
}

block_metadata_t *find_free_block(size_t size) {
    int start_class = get_size_class(size);

    // Look for the smallest size class that can accommodate the request
    for (int class = start_class; class < NUM_SIZE_CLASSES; class++) {
        block_metadata_t *current = free_lists[class];
        while (current) {
            if (current->size >= size) {
                return current;
            }
            current = current->next_free;
        }
    }
    
    return NULL;
}

void split_block(block_metadata_t *block, size_t size) {
    size_t min_split_size = sizeof(block_metadata_t) + 16;    
    if (block->size >= size + min_split_size) {
        size_t remainder_size = block->size - size - sizeof(block_metadata_t);
        
        block_metadata_t *new_block = (block_metadata_t *)((char *)block + sizeof(block_metadata_t) + size);
        new_block->size = remainder_size;
        new_block->is_free = true;
        
        block->size = size;
        
        add_to_free_list(new_block);
    }
}

void allocator_init(void *newbase) {
    base = newbase;
    used = 0;
    // Clear all free lists
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        free_lists[i] = NULL;
    }
}

void allocator_reset() { 
    used = 0; 
    // Clear all free lists
    for (int i = 0; i < NUM_SIZE_CLASSES; i++) {
        free_lists[i] = NULL;
    }
}

void *mymalloc(size_t size) {
    if (size == 0) return NULL;
    
    block_metadata_t *reuse_block = find_free_block(size);
    if (reuse_block) {
        reuse_block->is_free = false;
        
        remove_from_free_list(reuse_block);
        
        split_block(reuse_block, size);
        
        return get_user_ptr(reuse_block);
    }
    
    block_metadata_t *new_block = (block_metadata_t *)(((char *)base) + used);
    new_block->size = size;
    new_block->is_free = false;
    new_block->next_free = NULL;
    new_block->prev_free = NULL;
    
    used += sizeof(block_metadata_t) + size;
    
    return get_user_ptr(new_block);
}

void myfree(void *ptr) {
    if (ptr == NULL) return;
    
    block_metadata_t *block = get_metadata(ptr);
    
    block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
    if (block == last_block) {
        used -= (sizeof(block_metadata_t) + block->size);
        return;
    }
    
    add_to_free_list(block);
    
    merge_adjacent_free_blocks(block);
}

void *myrealloc(void *ptr, size_t size) {
    if (size == 0) { 
        myfree(ptr); 
        return NULL; 
    }
    
    if (ptr == NULL) {
        return mymalloc(size);
    }
    
    block_metadata_t *block = get_metadata(ptr);
    
    if (size <= block->size) {
        block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
        
        if (block == last_block) {
            size_t size_diff = block->size - size;
            block->size = size;
            used -= size_diff;
        } else {
            split_block(block, size);
        }
        
        return ptr;
    }
    
    block_metadata_t *last_block = (block_metadata_t *)((char *)base + used - sizeof(block_metadata_t) - block->size);
    if (block == last_block) {
        size_t additional_space = size - block->size;
        
        block->size = size;
        used += additional_space;
        
        return ptr;
    }
    
    block_metadata_t *next_block = find_block_after(block);
    if (next_block && next_block->is_free && 
        block->size + sizeof(block_metadata_t) + next_block->size >= size) {
        remove_from_free_list(next_block);
        
        block->size += sizeof(block_metadata_t) + next_block->size;
        
        split_block(block, size);
        
        return ptr;
    }
    
    void *new_ptr = mymalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size); 
        myfree(ptr);
    }
    
    return new_ptr;
}