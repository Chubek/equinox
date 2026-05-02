#ifndef EQUINOX_REWRITE_H
#define EQUINOX_REWRITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "equinox/enode.h"
#include "equinox/egraph.h"

/* Forward declarations */
typedef struct rewrite_rule rewrite_rule_t;
typedef struct rewrite_system rewrite_system_t;
typedef struct pattern pattern_t;
typedef struct substitution substitution_t;

/* Pattern matching types */
typedef enum {
    PATTERN_VAR,        /* Variable pattern (e.g., ?x) */
    PATTERN_CONST,      /* Constant pattern (specific operator with no children) */
    PATTERN_TERM        /* Term pattern (operator with child patterns) */
} pattern_type_t;

/* Pattern structure for matching */
typedef struct pattern {
    pattern_type_t type;
    
    union {
        /* For PATTERN_VAR */
        struct {
            uint32_t var_id;    /* Variable identifier */
        } var;
        
        /* For PATTERN_CONST */
        struct {
            uint32_t op;        /* Operator ID */
        } constant;
        
        /* For PATTERN_TERM */
        struct {
            uint32_t op;        /* Operator ID */
            size_t arity;       /* Number of children */
            pattern_t** children; /* Child patterns */
        } term;
    } data;
} pattern_t;

/* Substitution mapping from variables to eclass IDs */
typedef struct substitution {
    uint32_t* var_ids;          /* Array of variable IDs */
    eclass_id_t* eclass_ids;    /* Corresponding eclass IDs */
    size_t size;                /* Number of bindings */
    size_t capacity;            /* Allocated capacity */
} substitution_t;

/* Condition function type for conditional rewriting */
typedef bool (*rewrite_condition_fn)(const egraph_t* egraph, 
                                     const substitution_t* subst,
                                     void* user_data);

/* Rewrite rule structure */
typedef struct rewrite_rule {
    uint32_t id;                    /* Unique rule ID */
    pattern_t* lhs;                 /* Left-hand side pattern */
    pattern_t* rhs;                 /* Right-hand side pattern */
    rewrite_condition_fn condition; /* Optional condition function */
    void* condition_data;           /* User data for condition */
    double priority;                /* Rule priority (higher = applied first) */
    bool bidirectional;             /* If true, also apply rhs -> lhs */
    char* name;                     /* Optional rule name for debugging */
} rewrite_rule_t;

/* Rewrite system managing multiple rules */
typedef struct rewrite_system {
    rewrite_rule_t** rules;     /* Array of rules */
    size_t rule_count;          /* Number of rules */
    size_t rule_capacity;       /* Allocated capacity */
    uint32_t next_rule_id;      /* Next available rule ID */
} rewrite_system_t;

/* Match result structure */
typedef struct match_result {
    const rewrite_rule_t* rule; /* Matched rule */
    eclass_id_t matched_eclass; /* Eclass where match occurred */
    substitution_t* subst;      /* Variable substitution */
} match_result_t;

/* Pattern creation and destruction */
pattern_t* pattern_create_var(uint32_t var_id);
pattern_t* pattern_create_const(uint32_t op);
pattern_t* pattern_create_term(uint32_t op, size_t arity, pattern_t** children);
void pattern_destroy(pattern_t* pattern);
pattern_t* pattern_clone(const pattern_t* pattern);
char* pattern_to_string(const pattern_t* pattern);

/* Substitution operations */
substitution_t* substitution_create(void);
void substitution_destroy(substitution_t* subst);
void substitution_clear(substitution_t* subst);
bool substitution_add(substitution_t* subst, uint32_t var_id, eclass_id_t eclass_id);
bool substitution_get(const substitution_t* subst, uint32_t var_id, eclass_id_t* out_eclass_id);
bool substitution_contains(const substitution_t* subst, uint32_t var_id);
substitution_t* substitution_clone(const substitution_t* subst);
char* substitution_to_string(const substitution_t* subst);

/* Rewrite rule creation and destruction */
rewrite_rule_t* rewrite_rule_create(pattern_t* lhs, pattern_t* rhs);
rewrite_rule_t* rewrite_rule_create_conditional(pattern_t* lhs, 
                                                pattern_t* rhs,
                                                rewrite_condition_fn condition,
                                                void* condition_data);
void rewrite_rule_destroy(rewrite_rule_t* rule);
void rewrite_rule_set_name(rewrite_rule_t* rule, const char* name);
void rewrite_rule_set_priority(rewrite_rule_t* rule, double priority);
void rewrite_rule_set_bidirectional(rewrite_rule_t* rule, bool bidirectional);
char* rewrite_rule_to_string(const rewrite_rule_t* rule);

/* Rewrite system creation and management */
rewrite_system_t* rewrite_system_create(void);
void rewrite_system_destroy(rewrite_system_t* system);
uint32_t rewrite_system_add_rule(rewrite_system_t* system, rewrite_rule_t* rule);
bool rewrite_system_remove_rule(rewrite_system_t* system, uint32_t rule_id);
rewrite_rule_t* rewrite_system_get_rule(const rewrite_system_t* system, uint32_t rule_id);
size_t rewrite_system_rule_count(const rewrite_system_t* system);
void rewrite_system_clear(rewrite_system_t* system);

/* Pattern matching */
bool pattern_match(const pattern_t* pattern, 
                   const egraph_t* egraph,
                   eclass_id_t eclass_id,
                   substitution_t* subst);

bool pattern_match_node(const pattern_t* pattern,
                        const egraph_t* egraph,
                        const enode_t* node,
                        substitution_t* subst);

/* Apply pattern with substitution to create new term */
eclass_id_t pattern_apply(const pattern_t* pattern,
                          egraph_t* egraph,
                          const substitution_t* subst);

/* Find all matches for a rule in the egraph */
typedef struct match_list {
    match_result_t* matches;
    size_t count;
    size_t capacity;
} match_list_t;

match_list_t* rewrite_rule_find_matches(const rewrite_rule_t* rule, 
                                        const egraph_t* egraph);
void match_list_destroy(match_list_t* list);

/* Apply a single rewrite rule once (returns number of rewrites applied) */
size_t rewrite_rule_apply_once(const rewrite_rule_t* rule, egraph_t* egraph);

/* Apply a single rewrite rule exhaustively until saturation */
size_t rewrite_rule_apply_exhaustive(const rewrite_rule_t* rule, egraph_t* egraph);

/* Apply all rules in the system once */
size_t rewrite_system_apply_once(const rewrite_system_t* system, egraph_t* egraph);

/* Apply all rules exhaustively until saturation (fixed point) */
size_t rewrite_system_apply_exhaustive(const rewrite_system_t* system, 
                                       egraph_t* egraph,
                                       size_t max_iterations);

/* Rewrite statistics */
typedef struct rewrite_stats {
    size_t total_matches;       /* Total matches found */
    size_t total_rewrites;      /* Total rewrites applied */
    size_t iterations;          /* Number of iterations */
    size_t* rule_applications;  /* Per-rule application counts */
    size_t rule_count;          /* Number of rules tracked */
} rewrite_stats_t;

rewrite_stats_t* rewrite_stats_create(size_t rule_count);
void rewrite_stats_destroy(rewrite_stats_t* stats);
void rewrite_stats_reset(rewrite_stats_t* stats);
char* rewrite_stats_to_string(const rewrite_stats_t* stats);

/* Apply rules with statistics tracking */
size_t rewrite_system_apply_with_stats(const rewrite_system_t* system,
                                       egraph_t* egraph,
                                       size_t max_iterations,
                                       rewrite_stats_t* stats);

#ifdef __cplusplus
}
#endif

#endif /* EQUINOX_REWRITE_H */
