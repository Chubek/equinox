#include "redvisage/rewrite.h"
#include "redvisage/opcodes.h"

#include <stdlib.h>

static eqx_rewrite_rule_t* make_mul_two_to_add(void) {
    eqx_pattern_t* lhs_children[2];
    eqx_pattern_t* rhs_children[2];

    lhs_children[0] = eqx_pattern_var("x");
    lhs_children[1] = eqx_pattern_var("x");
    rhs_children[0] = eqx_pattern_var("x");
    rhs_children[1] = eqx_pattern_var("x");

    return eqx_rewrite_rule_create(
        "mul-two-to-add",
        eqx_pattern_app(REDVISAGE_EOP_MUL, lhs_children, 2),
        eqx_pattern_app(REDVISAGE_EOP_ADD, rhs_children, 2),
        NULL);
}

static eqx_rewrite_rule_t* make_gamma_true_fold(void) {
    eqx_pattern_t* lhs_children[4];

    lhs_children[0] = eqx_pattern_var("has_false");
    lhs_children[1] = eqx_pattern_app(redvisage_encode_const_int(1), NULL, 0);
    lhs_children[2] = eqx_pattern_var("true_val");
    lhs_children[3] = eqx_pattern_var("false_val");

    return eqx_rewrite_rule_create(
        "gamma-cond-true",
        eqx_pattern_app(REDVISAGE_EOP_GAMMA, lhs_children, 4),
        eqx_pattern_var("true_val"),
        NULL);
}

static eqx_rewrite_rule_t* make_gamma_false_fold(void) {
    eqx_pattern_t* lhs_children[4];

    lhs_children[0] = eqx_pattern_var("has_false");
    lhs_children[1] = eqx_pattern_app(redvisage_encode_const_int(0), NULL, 0);
    lhs_children[2] = eqx_pattern_var("true_val");
    lhs_children[3] = eqx_pattern_var("false_val");

    return eqx_rewrite_rule_create(
        "gamma-cond-false",
        eqx_pattern_app(REDVISAGE_EOP_GAMMA, lhs_children, 4),
        eqx_pattern_var("false_val"),
        NULL);
}

static eqx_rewrite_rule_t* make_theta_invariant_passthrough(void) {
    eqx_pattern_t* lhs_children[4];

    lhs_children[0] = eqx_pattern_var("carried_n");
    lhs_children[1] = eqx_pattern_var("arg_n");
    lhs_children[2] = eqx_pattern_var("v");
    lhs_children[3] = eqx_pattern_var("v");

    return eqx_rewrite_rule_create(
        "theta-invariant-passthrough",
        eqx_pattern_app(REDVISAGE_EOP_THETA, lhs_children, 4),
        eqx_pattern_var("v"),
        NULL);
}

eqx_rewrite_rule_t** redvisage_default_rules(size_t* out_count) {
    eqx_rewrite_rule_t** rules = (eqx_rewrite_rule_t**)calloc(4, sizeof(*rules));
    size_t count = 0;

    if (!rules) {
        return NULL;
    }

    rules[count++] = make_mul_two_to_add();
    rules[count++] = make_gamma_true_fold();
    rules[count++] = make_gamma_false_fold();
    rules[count++] = make_theta_invariant_passthrough();

    for (size_t i = 0; i < count; ++i) {
        if (!rules[i]) {
            redvisage_default_rules_destroy(rules, count);
            return NULL;
        }
    }

    if (out_count) {
        *out_count = count;
    }
    return rules;
}

void redvisage_default_rules_destroy(eqx_rewrite_rule_t** rules, size_t rule_count) {
    size_t i;
    if (!rules) return;
    for (i = 0; i < rule_count; ++i) {
        eqx_rewrite_rule_destroy(rules[i]);
    }
    free(rules);
}
