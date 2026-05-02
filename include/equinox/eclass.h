#ifndef EQUINOX_ECLASS_H
#define EQUINOX_ECLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct enode enode_t;
typedef uint32_t eclass_id_t;

/* Invalid eclass ID constant */
#define ECLASS_ID_INVALID ((eclass_id_t)-1)

/* EClass represents an equivalence class of terms */
typedef struct eclass {
    /* Unique identifier for this eclass */
    eclass_id_t id;
    
    /* Array of enodes in this eclass */
    enode_t** nodes;
    size_t node_count;
    size_t node_capacity;
    
    /* Parent eclasses that reference this eclass */
    eclass_id_t* parents;
    size_t parent_count;
    size_t parent_capacity;
    
    /* User-defined data associated with this eclass */
    void* user_data;
    
    /* Cost for extraction (used in cost-based extraction) */
    double cost;
    
    /* Best representative enode for extraction */
    enode_t* best_node;
} eclass_t;

/* EClass creation and destruction */
eclass_t* eclass_create(eclass_id_t id);
void eclass_destroy(eclass_t* eclass);

/* EClass operations */
void eclass_add_node(eclass_t* eclass, enode_t* node);
void eclass_add_parent(eclass_t* eclass, eclass_id_t parent_id);
bool eclass_contains_node(const eclass_t* eclass, const enode_t* node);

/* Merge two eclasses - returns the merged eclass */
eclass_t* eclass_merge(eclass_t* a, eclass_t* b);

/* Accessors */
eclass_id_t eclass_get_id(const eclass_t* eclass);
size_t eclass_get_node_count(const eclass_t* eclass);
enode_t* eclass_get_node(const eclass_t* eclass, size_t index);
size_t eclass_get_parent_count(const eclass_t* eclass);
eclass_id_t eclass_get_parent(const eclass_t* eclass, size_t index);

/* User data */
void eclass_set_user_data(eclass_t* eclass, void* data);
void* eclass_get_user_data(const eclass_t* eclass);

/* Cost and extraction */
void eclass_set_cost(eclass_t* eclass, double cost);
double eclass_get_cost(const eclass_t* eclass);
void eclass_set_best_node(eclass_t* eclass, enode_t* node);
enode_t* eclass_get_best_node(const eclass_t* eclass);

/* Find a node with specific operator in this eclass */
enode_t* eclass_find_node_by_op(const eclass_t* eclass, uint32_t op);

/* Iterator support */
typedef struct eclass_node_iterator {
    const eclass_t* eclass;
    size_t index;
} eclass_node_iterator_t;

eclass_node_iterator_t eclass_node_iterator_begin(const eclass_t* eclass);
bool eclass_node_iterator_has_next(const eclass_node_iterator_t* iter);
enode_t* eclass_node_iterator_next(eclass_node_iterator_t* iter);

/* String representation for debugging */
char* eclass_to_string(const eclass_t* eclass);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_ECLASS_H */
