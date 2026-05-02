#ifndef EQUINOX_ENODE_H
#define EQUINOX_ENODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct enode enode_t;
typedef struct eclass eclass_t;
typedef uint32_t eclass_id_t;

/* ENode represents a term node in the e-graph */
struct enode {
    /* Operator/symbol identifier */
    uint32_t op;
    
    /* Number of children */
    size_t arity;
    
    /* Array of child eclass IDs */
    eclass_id_t* children;
    
    /* Parent eclass this enode belongs to */
    eclass_id_t eclass;
    
    /* Hash value for hashconsing */
    uint64_t hash;
    
    /* User data pointer */
    void* user_data;
};

/* ENode creation and destruction */
enode_t* enode_create(uint32_t op, size_t arity, const eclass_id_t* children);
void enode_destroy(enode_t* node);

/* ENode operations */
uint64_t enode_hash(const enode_t* node);
bool enode_equal(const enode_t* a, const enode_t* b);
enode_t* enode_clone(const enode_t* node);

/* Accessors */
uint32_t enode_get_op(const enode_t* node);
size_t enode_get_arity(const enode_t* node);
eclass_id_t enode_get_child(const enode_t* node, size_t index);
eclass_id_t enode_get_eclass(const enode_t* node);

/* Setters */
void enode_set_eclass(enode_t* node, eclass_id_t eclass);
void enode_set_user_data(enode_t* node, void* data);
void* enode_get_user_data(const enode_t* node);

/* Canonicalization - replace children with canonical eclass IDs */
void enode_canonicalize(enode_t* node, eclass_id_t* (*find_fn)(eclass_id_t));

/* String representation for debugging */
char* enode_to_string(const enode_t* node);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_ENODE_H */
