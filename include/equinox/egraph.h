#ifndef EQUINOX_EGRAPH_H
#define EQUINOX_EGRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "eclass.h"
#include "enode.h"

/* Forward declarations */
typedef struct egraph egraph_t;
typedef struct unionfind unionfind_t;
typedef struct hashcons hashcons_t;

/* EGraph configuration */
typedef struct egraph_config {
    /* Initial capacity for eclasses */
    size_t initial_eclass_capacity;
    
    /* Initial capacity for hashcons table */
    size_t initial_hashcons_capacity;
    
    /* Enable automatic congruence closure */
    bool enable_congruence_closure;
    
    /* Enable automatic rebuilding after union operations */
    bool enable_auto_rebuild;
    
    /* Maximum number of rebuild iterations */
    size_t max_rebuild_iterations;
} egraph_config_t;

/* Default configuration */
egraph_config_t egraph_config_default(void);

/* EGraph structure */
typedef struct egraph {
    /* Union-find data structure for eclass representatives */
    unionfind_t* unionfind;
    
    /* Hashcons table for structural sharing */
    hashcons_t* hashcons;
    
    /* Array of all eclasses */
    eclass_t** eclasses;
    size_t eclass_count;
    size_t eclass_capacity;
    
    /* Worklist for pending congruence closure */
    eclass_id_t* worklist;
    size_t worklist_size;
    size_t worklist_capacity;
    
    /* Configuration */
    egraph_config_t config;
    
    /* Statistics */
    size_t union_count;
    size_t rebuild_count;
    size_t node_count;
} egraph_t;

/* EGraph creation and destruction */
egraph_t* egraph_create(void);
egraph_t* egraph_create_with_config(const egraph_config_t* config);
void egraph_destroy(egraph_t* egraph);

/* Add a term to the egraph - returns the eclass ID */
eclass_id_t egraph_add(egraph_t* egraph, uint32_t op, const eclass_id_t* children, size_t arity);

/* Merge two eclasses - returns the canonical eclass ID */
eclass_id_t egraph_union(egraph_t* egraph, eclass_id_t a, eclass_id_t b);

/* Find the canonical representative of an eclass */
eclass_id_t egraph_find(egraph_t* egraph, eclass_id_t id);

/* Check if two eclasses are equivalent */
bool egraph_equiv(egraph_t* egraph, eclass_id_t a, eclass_id_t b);

/* Rebuild the egraph to restore congruence closure invariants */
void egraph_rebuild(egraph_t* egraph);

/* Get an eclass by ID */
eclass_t* egraph_get_eclass(egraph_t* egraph, eclass_id_t id);

/* Lookup a term in the hashcons table - returns ECLASS_ID_INVALID if not found */
eclass_id_t egraph_lookup(egraph_t* egraph, uint32_t op, const eclass_id_t* children, size_t arity);

/* Canonicalize an enode (update children to canonical representatives) */
void egraph_canonicalize(egraph_t* egraph, enode_t* node);

/* Statistics */
size_t egraph_get_eclass_count(const egraph_t* egraph);
size_t egraph_get_node_count(const egraph_t* egraph);
size_t egraph_get_union_count(const egraph_t* egraph);
size_t egraph_get_rebuild_count(const egraph_t* egraph);

/* Iterator support for all eclasses */
typedef struct egraph_eclass_iterator {
    const egraph_t* egraph;
    size_t index;
} egraph_eclass_iterator_t;

egraph_eclass_iterator_t egraph_eclass_iterator_begin(const egraph_t* egraph);
bool egraph_eclass_iterator_has_next(const egraph_eclass_iterator_t* iter);
eclass_t* egraph_eclass_iterator_next(egraph_eclass_iterator_t* iter);

/* Clear the egraph (remove all eclasses and nodes) */
void egraph_clear(egraph_t* egraph);

/* String representation for debugging */
char* egraph_to_string(const egraph_t* egraph);

/* Dot graph export for visualization */
char* egraph_to_dot(const egraph_t* egraph);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_EGRAPH_H */
