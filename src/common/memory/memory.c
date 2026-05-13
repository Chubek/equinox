#include "common/memory.h"

#include "klib/kalloc.h"

#include <stdlib.h>
#include <string.h>

typedef struct redvisage_arena_block {
    struct redvisage_arena_block* next;
    size_t used;
    size_t cap;
    unsigned char data[1];
} redvisage_arena_block_t;

struct redvisage_arena {
    size_t block_size;
    redvisage_arena_block_t* head;
};

static void* default_alloc(void* user, size_t size) {
    (void)user;
    return malloc(size);
}

static void default_free(void* user, void* ptr) {
    (void)user;
    free(ptr);
}

redvisage_allocator_t redvisage_allocator_default(void) {
    redvisage_allocator_t a;
    a.alloc = default_alloc;
    a.free = default_free;
    a.user = NULL;
    return a;
}

void* redvisage_malloc(size_t size) {
    return kmalloc(NULL, size);
}

void redvisage_free(void* ptr) {
    kfree(NULL, ptr);
}

static redvisage_arena_block_t* arena_block_create(size_t cap) {
    size_t bytes = sizeof(redvisage_arena_block_t) + cap;
    redvisage_arena_block_t* block = (redvisage_arena_block_t*)malloc(bytes);
    if (!block) return NULL;
    block->next = NULL;
    block->used = 0;
    block->cap = cap;
    return block;
}

redvisage_arena_t* redvisage_arena_create(size_t block_size) {
    redvisage_arena_t* arena = (redvisage_arena_t*)calloc(1, sizeof(*arena));
    if (!arena) return NULL;
    arena->block_size = block_size > 0 ? block_size : 4096;
    return arena;
}

void redvisage_arena_destroy(redvisage_arena_t* arena) {
    redvisage_arena_block_t* b;
    redvisage_arena_block_t* next;
    if (!arena) return;
    b = arena->head;
    while (b) {
        next = b->next;
        free(b);
        b = next;
    }
    free(arena);
}

void* redvisage_arena_alloc(redvisage_arena_t* arena, size_t size) {
    redvisage_arena_block_t* block;
    size_t aligned;
    if (!arena || size == 0) return NULL;

    aligned = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    block = arena->head;
    if (!block || block->used + aligned > block->cap) {
        size_t cap = arena->block_size;
        if (aligned > cap) cap = aligned;
        block = arena_block_create(cap);
        if (!block) return NULL;
        block->next = arena->head;
        arena->head = block;
    }

    void* ptr = block->data + block->used;
    block->used += aligned;
    return ptr;
}

void redvisage_arena_reset(redvisage_arena_t* arena) {
    redvisage_arena_block_t* b;
    if (!arena) return;
    for (b = arena->head; b != NULL; b = b->next) {
        b->used = 0;
    }
}
