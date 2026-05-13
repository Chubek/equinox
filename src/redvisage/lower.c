#include "redvisage/lower.h"
#include "redvisage/opcodes.h"

#include "klib/khash.h"
#include <stdlib.h>

KHASH_MAP_INIT_INT(node2class, eqx_eclass_id_t)

static uint32_t redvisage_op_to_eop(rvsdg_op_t op) {
    switch (op) {
        case RVSDG_OP_ADD: return REDVISAGE_EOP_ADD;
        case RVSDG_OP_MUL: return REDVISAGE_EOP_MUL;
        case RVSDG_OP_LESS_THAN: return REDVISAGE_EOP_LT;
        case RVSDG_OP_PRINT: return REDVISAGE_EOP_PRINT;
        case RVSDG_OP_YIELD: return REDVISAGE_EOP_YIELD;
        default: return REDVISAGE_EOP_ADD;
    }
}

static eqx_eclass_id_t lower_node(rvsdg_node_t* node, eqx_egraph_t* eg, khash_t(node2class)* memo) {
    int ret;
    khiter_t it;
    eqx_eclass_id_t* children;
    size_t i;

    if (!node || !eg || !memo) return EQX_ECLASS_ID_INVALID;

    it = kh_get(node2class, memo, node->id);
    if (it != kh_end(memo)) {
        return kh_value(memo, it);
    }

    children = 0;
    if (kv_size(node->inputs) > 0) {
        children = (eqx_eclass_id_t*)calloc(kv_size(node->inputs), sizeof(*children));
        if (!children) return EQX_ECLASS_ID_INVALID;
        for (i = 0; i < kv_size(node->inputs); ++i) {
            children[i] = lower_node(kv_A(node->inputs, i), eg, memo);
        }
    }

    uint32_t op = REDVISAGE_EOP_CONST_BASE;
    switch (node->kind) {
        case RVSDG_NODE_CONST_INT: op = redvisage_encode_const_int(node->data.const_int); break;
        case RVSDG_NODE_VAR: op = REDVISAGE_EOP_VAR; break;
        case RVSDG_NODE_OP: op = redvisage_op_to_eop(node->data.op); break;
        case RVSDG_NODE_GAMMA: op = REDVISAGE_EOP_GAMMA; break;
        case RVSDG_NODE_THETA: op = REDVISAGE_EOP_THETA; break;
        case RVSDG_NODE_STATE: op = REDVISAGE_EOP_STATE_INIT; break;
        case RVSDG_NODE_EXTRACT: op = REDVISAGE_EOP_EXTRACT; break;
    }

    eqx_eclass_id_t id = EQX_ECLASS_ID_INVALID;
    if (node->kind == RVSDG_NODE_GAMMA) {
        eqx_eclass_id_t branch_children[4];
        size_t arity = 0;
        eqx_eclass_id_t false_present = eqx_egraph_add(eg, redvisage_encode_const_int(0), 0, NULL);
        if (node->data.gamma.false_region && node->data.gamma.false_region->yield_node) {
            false_present = eqx_egraph_add(eg, redvisage_encode_const_int(1), 0, NULL);
        }
        branch_children[arity++] = false_present;
        branch_children[arity++] = lower_node(node->data.gamma.cond, eg, memo);
        if (node->data.gamma.true_region && node->data.gamma.true_region->yield_node) {
            branch_children[arity++] = lower_node(node->data.gamma.true_region->yield_node, eg, memo);
        }
        if (node->data.gamma.false_region && node->data.gamma.false_region->yield_node) {
            branch_children[arity++] = lower_node(node->data.gamma.false_region->yield_node, eg, memo);
        }
        id = eqx_egraph_add(eg, op, arity, branch_children);
    } else if (node->kind == RVSDG_NODE_THETA) {
        size_t body_nodes = 0;
        eqx_eclass_id_t* loop_children = NULL;
        eqx_eclass_id_t carried_count = eqx_egraph_add(eg, redvisage_encode_const_int((int64_t)kv_size(node->inputs)), 0, NULL);
        eqx_eclass_id_t region_arg_count = eqx_egraph_add(eg, redvisage_encode_const_int(node->data.theta.body ? (int64_t)kv_size(node->data.theta.body->args) : 0), 0, NULL);
        if (node->data.theta.body) {
            body_nodes = kv_size(node->data.theta.body->args);
            if (node->data.theta.body->yield_node) {
                body_nodes += 1;
            }
        }
        loop_children = (eqx_eclass_id_t*)calloc(2 + kv_size(node->inputs) + body_nodes, sizeof(*loop_children));
        if (!loop_children) {
            free(children);
            return EQX_ECLASS_ID_INVALID;
        }
        size_t out = 0;
        loop_children[out++] = carried_count;
        loop_children[out++] = region_arg_count;
        for (i = 0; i < kv_size(node->inputs); ++i) {
            loop_children[out++] = children[i];
        }
        if (node->data.theta.body) {
            for (i = 0; i < kv_size(node->data.theta.body->args); ++i) {
                loop_children[out++] = lower_node(kv_A(node->data.theta.body->args, i), eg, memo);
            }
            if (node->data.theta.body->yield_node) {
                loop_children[out++] = lower_node(node->data.theta.body->yield_node, eg, memo);
            }
        }
        id = eqx_egraph_add(eg, op, out, loop_children);
        free(loop_children);
    } else {
        id = eqx_egraph_add(eg, op, kv_size(node->inputs), children);
    }
    free(children);

    it = kh_put(node2class, memo, node->id, &ret);
    if (it != kh_end(memo)) kh_value(memo, it) = id;
    return id;
}

eqx_eclass_id_t redvisage_lower_to_egraph(rvsdg_graph_t* graph, eqx_egraph_t* egraph) {
    khash_t(node2class)* memo;
    eqx_eclass_id_t root;
    if (!graph || !graph->root || !egraph) return EQX_ECLASS_ID_INVALID;

    memo = kh_init(node2class);
    if (!memo) return EQX_ECLASS_ID_INVALID;
    root = lower_node(graph->root, egraph, memo);
    kh_destroy(node2class, memo);
    return root;
}
