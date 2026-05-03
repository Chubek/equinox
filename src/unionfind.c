#include "equinox/unionfind.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Union-Find element */
typedef struct {
    eqx_eclass_id_t parent;
    size_t rank;
} uf_element_t;

/* Union-Find structure */
struct eqx_unionfind {
    uf_element_t *elements;
    size_t capacity;
    size_t size;
};

/* Creation and destruction */
eqx_unionfind_t *eqx_unionfind_create(size_t initial_capacity) {
    eqx_unionfind_t *uf = malloc(sizeof(eqx_unionfind_t));
    if (!uf) return NULL;

    uf->capacity = initial_capacity > 0 ? initial_capacity : 16;
    uf->elements = malloc(uf->capacity * sizeof(uf_element_t));
    if (!uf->elements) {
        free(uf);
        return NULL;
    }

    uf->size = 0;

    return uf;
}

void eqx_unionfind_destroy(eqx_unionfind_t *uf) {
    if (!uf) return;
    free(uf->elements);
    free(uf);
}

/* Set operations */
eqx_eclass_id_t eqx_unionfind_make_set(eqx_unionfind_t *uf) {
    assert(uf);

    /* Expand if needed */
    if (uf->size >= uf->capacity) {
        size_t new_capacity = uf->capacity * 2;
        uf_element_t *new_elements = realloc(uf->elements, new_capacity * sizeof(uf_element_t));
        if (!new_elements) return EQX_ECLASS_ID_INVALID;
        uf->elements = new_elements;
        uf->capacity = new_capacity;
    }

    eqx_eclass_id_t new_id = uf->size++;
    uf->elements[new_id].parent = new_id;
    uf->elements[new_id].rank = 0;

    return new_id;
}

eqx_eclass_id_t eqx_unionfind_find(eqx_unionfind_t *uf, eqx_eclass_id_t id) {
    assert(uf);
    if (id >= uf->size) return EQX_ECLASS_ID_INVALID;

    /* Find root with path compression */
    if (uf->elements[id].parent != id) {
        uf->elements[id].parent = eqx_unionfind_find(uf, uf->elements[id].parent);
    }

    return uf->elements[id].parent;
}

bool eqx_unionfind_union(eqx_unionfind_t *uf, eqx_eclass_id_t id1, eqx_eclass_id_t id2) {
    assert(uf);
    if (id1 >= uf->size || id2 >= uf->size) return false;

    eqx_eclass_id_t root1 = eqx_unionfind_find(uf, id1);
    eqx_eclass_id_t root2 = eqx_unionfind_find(uf, id2);

    if (root1 == root2) return true;

    /* Union by rank */
    if (uf->elements[root1].rank < uf->elements[root2].rank) {
        uf->elements[root1].parent = root2;
    } else if (uf->elements[root1].rank > uf->elements[root2].rank) {
        uf->elements[root2].parent = root1;
    } else {
        uf->elements[root2].parent = root1;
        uf->elements[root1].rank++;
    }

    return true;
}

bool eqx_unionfind_equiv(eqx_unionfind_t *uf, eqx_eclass_id_t id1, eqx_eclass_id_t id2) {
    assert(uf);
    if (id1 >= uf->size || id2 >= uf->size) return false;
    return eqx_unionfind_find(uf, id1) == eqx_unionfind_find(uf, id2);
}

/* Accessors */
size_t eqx_unionfind_size(const eqx_unionfind_t *uf) {
    assert(uf);
    return uf->size;
}

size_t eqx_unionfind_num_sets(const eqx_unionfind_t *uf) {
    assert(uf);

    size_t count = 0;
    for (size_t i = 0; i < uf->size; i++) {
        if (uf->elements[i].parent == i) {
            count++;
        }
    }

    return count;
}
