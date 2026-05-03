#include "equinox/rewrite.h"
#include "equinox/egraph.h"
#include "equinox/eclass.h"
#include "equinox/enode.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Pattern structure */
struct eqx_pattern {
    eqx_pattern_type_t type;
    union {
        struct {
            const char *name;
        } var;
        struct {
            eqx_operator_t op;
            eqx_pattern_t **children;
            size_t arity;
        } app;
    } data;
};

/* Substitution entry */
typedef struct subst_entry {
    const char *var_name;
    eqx_eclass_id_t eclass_id;
    struct subst_entry *next;
} subst_entry_t;

/* Substitution map */
typedef struct {
    subst_entry_t *head;
} substitution_t;

/* Rewrite rule structure */
struct eqx_rewrite_rule {
    const char *name;
    eqx_pattern_t *lhs;
    eqx_pattern_t *rhs;
    eqx_rewrite_condition_fn condition;
    void *condition_data;
};

/* Internal helpers */
static substitution_t *subst_create(void) {
    substitution_t *subst = malloc(sizeof(substitution_t));
    if (!subst) return NULL;
    subst->head = NULL;
    return subst;
}

static void subst_destroy(substitution_t *subst) {
    if (!subst) return;
    
    subst_entry_t *entry = subst->head;
    while (entry) {
        subst_entry_t *next = entry->next;
        free(entry);
        entry = next;
    }
    free(subst);
}

static bool subst_bind(substitution_t *subst, const char *var_name, eqx_eclass_id_t eclass_id) {
    /* Check if already bound */
    subst_entry_t *entry = subst->head;
    while (entry) {
        if (strcmp(entry->var_name, var_name) == 0) {
            /* Variable already bound - check consistency */
            return entry->eclass_id == eclass_id;
        }
        entry = entry->next;
    }
    
    /* Add new binding */
    subst_entry_t *new_entry = malloc(sizeof(subst_entry_t));
    if (!new_entry) return false;
    
    new_entry->var_name = var_name;
    new_entry->eclass_id = eclass_id;
    new_entry->next = subst->head;
    subst->head = new_entry;
    
    return true;
}

static bool subst_lookup(const substitution_t *subst, const char *var_name, eqx_eclass_id_t *out_id) {
    subst_entry_t *entry = subst->head;
    while (entry) {
        if (strcmp(entry->var_name, var_name) == 0) {
            *out_id = entry->eclass_id;
            return true;
        }
        entry = entry->next;
    }
    return false;
}

static substitution_t *subst_clone(const substitution_t *subst) {
    substitution_t *new_subst = subst_create();
    if (!new_subst) return NULL;
    
    subst_entry_t *entry = subst->head;
    while (entry) {
        if (!subst_bind(new_subst, entry->var_name, entry->eclass_id)) {
            subst_destroy(new_subst);
            return NULL;
        }
        entry = entry->next;
    }
    
    return new_subst;
}

/* Pattern matching */
static bool match_pattern_internal(
    const eqx_egraph_t *egraph,
    const eqx_pattern_t *pattern,
    eqx_eclass_id_t eclass_id,
    substitution_t *subst
) {
    assert(egraph);
    assert(pattern);
    assert(subst);
    
    if (pattern->type == EQX_PATTERN_VAR) {
        return subst_bind(subst, pattern->data.var.name, eclass_id);
    }
    
    /* Pattern is an application */
    eqx_eclass_t *eclass = eqx_egraph_get_eclass(egraph, eclass_id);
    if (!eclass) return false;
    
    /* Try to find matching node in e-class */
    eqx_enode_t *node = eqx_eclass_find_node(eclass, pattern->data.app.op, 
                                              pattern->data.app.arity);
    if (!node) return false;
    
    /* Match children recursively */
    for (size_t i = 0; i < pattern->data.app.arity; i++) {
        eqx_eclass_id_t child_id = eqx_enode_get_child(node, i);
        if (!match_pattern_internal(egraph, pattern->data.app.children[i], 
                                     child_id, subst)) {
            return false;
        }
    }
    
    return true;
}

/* Pattern instantiation */
static eqx_eclass_id_t instantiate_pattern(
    eqx_egraph_t *egraph,
    const eqx_pattern_t *pattern,
    const substitution_t *subst
) {
    assert(egraph);
    assert(pattern);
    assert(subst);
    
    if (pattern->type == EQX_PATTERN_VAR) {
        eqx_eclass_id_t eclass_id;
        if (!subst_lookup(subst, pattern->data.var.name, &eclass_id)) {
            return EQX_ECLASS_ID_INVALID;
        }
        return eclass_id;
    }
    
    /* Pattern is an application - instantiate children first */
    eqx_eclass_id_t *child_ids = malloc(pattern->data.app.arity * sizeof(eqx_eclass_id_t));
    if (!child_ids) return EQX_ECLASS_ID_INVALID;
    
    for (size_t i = 0; i < pattern->data.app.arity; i++) {
        child_ids[i] = instantiate_pattern(egraph, pattern->data.app.children[i], subst);
        if (child_ids[i] == EQX_ECLASS_ID_INVALID) {
            free(child_ids);
            return EQX_ECLASS_ID_INVALID;
        }
    }
    
    /* Add term to e-graph */
    eqx_eclass_id_t result = eqx_egraph_add(egraph, pattern->data.app.op, 
                                             child_ids, pattern->data.app.arity);
    free(child_ids);
    
    return result;
}

/* Pattern creation */
eqx_pattern_t *eqx_pattern_var(const char *name) {
    assert(name);
    
    eqx_pattern_t *pattern = malloc(sizeof(eqx_pattern_t));
    if (!pattern) return NULL;
    
    pattern->type = EQX_PATTERN_VAR;
    pattern->data.var.name = name;
    
    return pattern;
}

eqx_pattern_t *eqx_pattern_app(eqx_operator_t op, eqx_pattern_t **children, size_t arity) {
    assert(children || arity == 0);
    
    eqx_pattern_t *pattern = malloc(sizeof(eqx_pattern_t));
    if (!pattern) return NULL;
    
    pattern->type = EQX_PATTERN_APP;
    pattern->data.app.op = op;
    pattern->data.app.arity = arity;
    
    if (arity > 0) {
        pattern->data.app.children = malloc(arity * sizeof(eqx_pattern_t *));
        if (!pattern->data.app.children) {
            free(pattern);
            return NULL;
        }
        memcpy(pattern->data.app.children, children, arity * sizeof(eqx_pattern_t *));
    } else {
        pattern->data.app.children = NULL;
    }
    
    return pattern;
}

void eqx_pattern_destroy(eqx_pattern_t *pattern) {
    if (!pattern) return;
    
    if (pattern->type == EQX_PATTERN_APP) {
        for (size_t i = 0; i < pattern->data.app.arity; i++) {
            eqx_pattern_destroy(pattern->data.app.children[i]);
        }
        free(pattern->data.app.children);
    }
    
    free(pattern);
}

/* Pattern accessors */
eqx_pattern_type_t eqx_pattern_get_type(const eqx_pattern_t *pattern) {
    assert(pattern);
    return pattern->type;
}

const char *eqx_pattern_get_var_name(const eqx_pattern_t *pattern) {
    assert(pattern);
    assert(pattern->type == EQX_PATTERN_VAR);
    return pattern->data.var.name;
}

eqx_operator_t eqx_pattern_get_operator(const eqx_pattern_t *pattern) {
    assert(pattern);
    assert(pattern->type == EQX_PATTERN_APP);
    return pattern->data.app.op;
}

size_t eqx_pattern_get_arity(const eqx_pattern_t *pattern) {
    assert(pattern);
    assert(pattern->type == EQX_PATTERN_APP);
    return pattern->data.app.arity;
}

eqx_pattern_t *eqx_pattern_get_child(const eqx_pattern_t *pattern, size_t index) {
    assert(pattern);
    assert(pattern->type == EQX_PATTERN_APP);
    assert(index < pattern->data.app.arity);
    return pattern->data.app.children[index];
}

/* Rewrite rule creation */
eqx_rewrite_rule_t *eqx_rewrite_rule_create(
    const char *name,
    eqx_pattern_t *lhs,
    eqx_pattern_t *rhs,
    eqx_rewrite_condition_fn condition,
    void *condition_data
) {
    assert(name);
    assert(lhs);
    assert(rhs);
    
    eqx_rewrite_rule_t *rule = malloc(sizeof(eqx_rewrite_rule_t));
    if (!rule) return NULL;
    
    rule->name = name;
    rule->lhs = lhs;
    rule->rhs = rhs;
    rule->condition = condition;
    rule->condition_data = condition_data;
    
    return rule;
}

void eqx_rewrite_rule_destroy(eqx_rewrite_rule_t *rule) {
    if (!rule) return;
    
    eqx_pattern_destroy(rule->lhs);
    eqx_pattern_destroy(rule->rhs);
    free(rule);
}

/* Rewrite rule accessors */
const char *eqx_rewrite_rule_get_name(const eqx_rewrite_rule_t *rule) {
    assert(rule);
    return rule->name;
}

const eqx_pattern_t *eqx_rewrite_rule_get_lhs(const eqx_rewrite_rule_t *rule) {
    assert(rule);
    return rule->lhs;
}

const eqx_pattern_t *eqx_rewrite_rule_get_rhs(const eqx_rewrite_rule_t *rule) {
    assert(rule);
    return rule->rhs;
}

/* Pattern matching */
bool eqx_pattern_match(
    const eqx_egraph_t *egraph,
    const eqx_pattern_t *pattern,
    eqx_eclass_id_t eclass_id,
    eqx_match_callback_fn callback,
    void *user_data
) {
    assert(egraph);
    assert(pattern);
    assert(callback);
    
    substitution_t *subst = subst_create();
    if (!subst) return false;
    
    bool matched = match_pattern_internal(egraph, pattern, eclass_id, subst);
    
    if (matched) {
        callback(egraph, pattern, eclass_id, user_data);
    }
    
    subst_destroy(subst);
    return matched;
}

/* Rewrite application */
size_t eqx_rewrite_apply(
    eqx_egraph_t *egraph,
    const eqx_rewrite_rule_t *rule,
    eqx_eclass_id_t eclass_id
) {
    assert(egraph);
    assert(rule);
    
    size_t rewrites = 0;
    
    /* Try to match LHS pattern */
    substitution_t *subst = subst_create();
    if (!subst) return 0;
    
    if (match_pattern_internal(egraph, rule->lhs, eclass_id, subst)) {
        /* Check condition if present */
        if (rule->condition && !rule->condition(egraph, subst, rule->condition_data)) {
            subst_destroy(subst);
            return 0;
        }
        
        /* Instantiate RHS pattern */
        eqx_eclass_id_t rhs_id = instantiate_pattern(egraph, rule->rhs, subst);
        
        if (rhs_id != EQX_ECLASS_ID_INVALID) {
            /* Union LHS and RHS e-classes */
            eqx_egraph_union(egraph, eclass_id, rhs_id);
            rewrites = 1;
        }
    }
    
    subst_destroy(subst);
    return rewrites;
}

size_t eqx_rewrite_apply_all(
    eqx_egraph_t *egraph,
    const eqx_rewrite_rule_t *rule
) {
    assert(egraph);
    assert(rule);
    
    size_t total_rewrites = 0;
    
    /* Iterate over all e-classes */
    eqx_egraph_iter_t iter = eqx_egraph_iter_begin(egraph);
    while (eqx_egraph_iter_has_next(iter)) {
        eqx_eclass_id_t eclass_id = eqx_egraph_iter_next(iter);
        total_rewrites += eqx_rewrite_apply(egraph, rule, eclass_id);
    }
    eqx_egraph_iter_end(iter);
    
    return total_rewrites;
}

/* Multi-rule rewriting */
size_t eqx_rewrite_apply_rules(
    eqx_egraph_t *egraph,
    eqx_rewrite_rule_t **rules,
    size_t num_rules,
    size_t max_iterations
) {
    assert(egraph);
    assert(rules || num_rules == 0);
    
    size_t total_rewrites = 0;
    size_t iteration = 0;
    
    while (iteration < max_iterations) {
        size_t iteration_rewrites = 0;
        
        /* Apply each rule to all e-classes */
        for (size_t i = 0; i < num_rules; i++) {
            iteration_rewrites += eqx_rewrite_apply_all(egraph, rules[i]);
        }
        
        /* Rebuild e-graph to restore invariants */
        eqx_egraph_rebuild(egraph);
        
        total_rewrites += iteration_rewrites;
        
        /* Stop if no rewrites occurred */
        if (iteration_rewrites == 0) {
            break;
        }
        
        iteration++;
    }
    
    return total_rewrites;
}
