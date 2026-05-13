#ifndef EQX_COMMON_MEMORY_H
#define EQX_COMMON_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redvisage_arena redvisage_arena_t;

typedef void* (*redvisage_alloc_fn)(void* user, size_t size);
typedef void (*redvisage_free_fn)(void* user, void* ptr);

typedef struct redvisage_allocator {
    redvisage_alloc_fn alloc;
    redvisage_free_fn free;
    void* user;
} redvisage_allocator_t;

redvisage_allocator_t redvisage_allocator_default(void);
void* redvisage_malloc(size_t size);
void redvisage_free(void* ptr);

redvisage_arena_t* redvisage_arena_create(size_t block_size);
void redvisage_arena_destroy(redvisage_arena_t* arena);
void* redvisage_arena_alloc(redvisage_arena_t* arena, size_t size);
void redvisage_arena_reset(redvisage_arena_t* arena);

#ifdef __cplusplus
}
#endif

#endif
