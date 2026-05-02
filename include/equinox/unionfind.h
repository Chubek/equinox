#ifndef EQUINOX_UNIONFIND_H
#define EQUINOX_UNIONFIND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Forward declaration */
typedef struct unionfind unionfind_t;

/* Union-Find element ID type */
typedef uint32_t uf_id_t;

/* Invalid ID constant */
#define UF_ID_INVALID ((uf_id_t)-1)

/* Union-Find structure with path compression and union by rank */
typedef struct unionfind {
    /* Parent pointers (parent[i] is the parent of element i) */
    uf_id_t* parent;
    
    /* Rank for union by rank heuristic */
    uint32_t* rank;
    
    /* Number of elements */
    size_t size;
    
    /* Capacity of the arrays */
    size_t capacity;
    
    /* Number of distinct sets */
    size_t set_count;
} unionfind_t;

/* Creation and destruction */
unionfind_t* unionfind_create(size_t initial_capacity);
void unionfind_destroy(unionfind_t* uf);

/* Add a new element - returns its ID */
uf_id_t unionfind_make_set(unionfind_t* uf);

/* Find the representative of a set (with path compression) */
uf_id_t unionfind_find(unionfind_t* uf, uf_id_t id);

/* Union two sets - returns the new representative */
uf_id_t unionfind_union(unionfind_t* uf, uf_id_t a, uf_id_t b);

/* Check if two elements are in the same set */
bool unionfind_equiv(unionfind_t* uf, uf_id_t a, uf_id_t b);

/* Get the number of elements */
size_t unionfind_size(const unionfind_t* uf);

/* Get the number of distinct sets */
size_t unionfind_set_count(const unionfind_t* uf);

/* Clear all elements */
void unionfind_clear(unionfind_t* uf);

/* Reserve capacity for at least n elements */
void unionfind_reserve(unionfind_t* uf, size_t n);

/* Check if an ID is valid */
bool unionfind_is_valid_id(const unionfind_t* uf, uf_id_t id);

/* String representation for debugging */
char* unionfind_to_string(const unionfind_t* uf);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_UNIONFIND_H */
