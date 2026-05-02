#ifndef EQUINOX_HASHCONS_H
#define EQUINOX_HASHCONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "equinox/enode.h"

/* Forward declarations */
typedef struct hashcons hashcons_t;
typedef struct hashcons_entry hashcons_entry_t;

/* Hash function type for enodes */
typedef uint64_t (*hashcons_hash_fn)(const enode_t* node);

/* Equality function type for enodes */
typedef bool (*hashcons_eq_fn)(const enode_t* a, const enode_t* b);

/* Hash table entry */
typedef struct hashcons_entry {
    enode_t* node;              /* The stored node */
    eclass_id_t eclass_id;      /* Associated eclass ID */
    uint64_t hash;              /* Cached hash value */
    struct hashcons_entry* next; /* Next entry in chain (for collision resolution) */
} hashcons_entry_t;

/* Hashcons table structure */
typedef struct hashcons {
    /* Array of buckets (each bucket is a linked list) */
    hashcons_entry_t** buckets;
    
    /* Number of buckets */
    size_t bucket_count;
    
    /* Number of entries */
    size_t entry_count;
    
    /* Load factor threshold for resizing (e.g., 0.75) */
    double load_factor;
    
    /* Hash function */
    hashcons_hash_fn hash_fn;
    
    /* Equality function */
    hashcons_eq_fn eq_fn;
} hashcons_t;

/* Creation and destruction */
hashcons_t* hashcons_create(size_t initial_capacity, 
                            hashcons_hash_fn hash_fn,
                            hashcons_eq_fn eq_fn);
void hashcons_destroy(hashcons_t* hc);

/* Insert or lookup a node
 * If the node exists, returns the existing eclass_id
 * If not, inserts it with the given eclass_id and returns that ID
 * The node is cloned internally if inserted
 */
eclass_id_t hashcons_insert(hashcons_t* hc, const enode_t* node, eclass_id_t eclass_id);

/* Lookup a node - returns ECLASS_ID_INVALID if not found */
eclass_id_t hashcons_lookup(const hashcons_t* hc, const enode_t* node);

/* Check if a node exists in the table */
bool hashcons_contains(const hashcons_t* hc, const enode_t* node);

/* Remove a node from the table - returns true if found and removed */
bool hashcons_remove(hashcons_t* hc, const enode_t* node);

/* Update the eclass_id for a node - returns true if found and updated */
bool hashcons_update(hashcons_t* hc, const enode_t* node, eclass_id_t new_eclass_id);

/* Get the number of entries */
size_t hashcons_size(const hashcons_t* hc);

/* Get the number of buckets */
size_t hashcons_bucket_count(const hashcons_t* hc);

/* Get the current load factor */
double hashcons_load_factor(const hashcons_t* hc);

/* Clear all entries */
void hashcons_clear(hashcons_t* hc);

/* Manually trigger a rehash with a new bucket count */
void hashcons_rehash(hashcons_t* hc, size_t new_bucket_count);

/* Reserve capacity for at least n entries */
void hashcons_reserve(hashcons_t* hc, size_t n);

/* Iterator support */
typedef struct hashcons_iterator {
    const hashcons_t* hc;
    size_t bucket_index;
    hashcons_entry_t* current_entry;
} hashcons_iterator_t;

/* Initialize an iterator */
hashcons_iterator_t hashcons_iterator_begin(const hashcons_t* hc);

/* Check if iterator has more elements */
bool hashcons_iterator_has_next(const hashcons_iterator_t* it);

/* Advance iterator and return current entry */
const hashcons_entry_t* hashcons_iterator_next(hashcons_iterator_t* it);

/* Statistics for debugging */
typedef struct hashcons_stats {
    size_t entry_count;
    size_t bucket_count;
    size_t empty_buckets;
    size_t max_chain_length;
    double avg_chain_length;
    double load_factor;
} hashcons_stats_t;

hashcons_stats_t hashcons_get_stats(const hashcons_t* hc);

/* String representation for debugging */
char* hashcons_to_string(const hashcons_t* hc);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_HASHCONS_H */
