#include "redvisage/extract.h"
#include "redvisage/opcodes.h"

#include "equinox/eclass.h"
#include "klib/khash.h"

#include <float.h>
#include <stdlib.h>

KHASH_MAP_INIT_INT(extracted, rvsdg_node_t*)
KHASH_MAP_INIT_INT(cost_cache, double)
KHASH_SET_INIT_INT(visiting)

static double opcode_cost(uint32_t op) {
    if (redvisage_is_const_opcode(op)) return 0.1;
    switch (op) {
        case REDVISAGE_EOP_VAR: return 0.2;
        case REDVISAGE_EOP_STATE_INIT: return 0.2;
        case REDVISAGE_EOP_ADD:
        case REDVISAGE_EOP_MUL:
        case REDVISAGE_EOP_LT: return 1.0;
        case REDVISAGE_EOP_PRINT: return 2.0;
        case REDVISAGE_EOP_YIELD: return 0.6;
        case REDVISAGE_EOP_EXTRACT: return 0.5;
        case REDVISAGE_EOP_GAMMA: return 3.0;
        case REDVISAGE_EOP_THETA: return 4.0;
        default: return 1.5;
    }
}

static int extract_const_from_eclass(eqx_egraph_t* eg, eqx_eclass_id_t id, int64_t* out) {
    eqx_eclass_t* ec = eqx_egraph_get_eclass(eg, id);
    if (!ec) return 0;
    for (eqx_enode_t* n = ec->nodes; n != NULL; n = n->next_in_class) {
        if (redvisage_is_const_opcode(n->op)) {
            *out = redvisage_decode_const_int(n->op);
            return 1;
        }
    }
    return 0;
}

static double compute_eclass_cost(eqx_egraph_t* eg, eqx_eclass_id_t id, khash_t(cost_cache)* cache, khash_t(visiting)* visiting);

static double compute_enode_cost(eqx_egraph_t* eg, const eqx_enode_t* n, khash_t(cost_cache)* cache, khash_t(visiting)* visiting) {
    double cost = opcode_cost(n->op);
    for (size_t i = 0; i < n->arity; ++i) {
        double child_cost = compute_eclass_cost(eg, n->children[i], cache, visiting);
        if (child_cost >= DBL_MAX / 8.0) {
            return DBL_MAX;
        }
        cost += child_cost;
    }
    return cost;
}

static double compute_eclass_cost(eqx_egraph_t* eg, eqx_eclass_id_t id, khash_t(cost_cache)* cache, khash_t(visiting)* visiting) {
    khiter_t it = kh_get(cost_cache, cache, id);
    if (it != kh_end(cache)) {
        return kh_value(cache, it);
    }

    it = kh_get(visiting, visiting, id);
    if (it != kh_end(visiting)) {
        return DBL_MAX;
    }

    int ret = 0;
    it = kh_put(visiting, visiting, id, &ret);
    (void)it;

    eqx_eclass_t* ec = eqx_egraph_get_eclass(eg, id);
    double best = DBL_MAX;
    if (ec) {
        for (eqx_enode_t* n = ec->nodes; n != NULL; n = n->next_in_class) {
            double c = compute_enode_cost(eg, n, cache, visiting);
            if (c < best) {
                best = c;
            }
        }
    }

    kh_del(visiting, visiting, kh_get(visiting, visiting, id));

    it = kh_put(cost_cache, cache, id, &ret);
    if (it != kh_end(cache)) {
        kh_value(cache, it) = best;
    }
    return best;
}

static const eqx_enode_t* choose_best_node(eqx_egraph_t* eg, eqx_eclass_id_t id, khash_t(cost_cache)* cache, khash_t(visiting)* visiting) {
    eqx_eclass_t* ec = eqx_egraph_get_eclass(eg, id);
    const eqx_enode_t* best = NULL;
    double best_cost = DBL_MAX;
    if (!ec) return NULL;

    for (eqx_enode_t* n = ec->nodes; n != NULL; n = n->next_in_class) {
        double c = compute_enode_cost(eg, n, cache, visiting);
        if (c < best_cost) {
            best_cost = c;
            best = n;
        }
    }
    return best;
}

static rvsdg_node_t* extract_eclass(rvsdg_graph_t* g, eqx_egraph_t* eg, eqx_eclass_id_t id,
                                    khash_t(extracted)* memo, khash_t(cost_cache)* cost_cache,
                                    khash_t(visiting)* visiting) {
    khiter_t it = kh_get(extracted, memo, id);
    if (it != kh_end(memo)) return kh_value(memo, it);

    const eqx_enode_t* best = choose_best_node(eg, id, cost_cache, visiting);
    if (!best) return NULL;

    if (redvisage_is_const_opcode(best->op)) {
        rvsdg_node_t* n = redvisage_make_const_int(g, redvisage_decode_const_int(best->op));
        int ret;
        it = kh_put(extracted, memo, id, &ret);
        kh_value(memo, it) = n;
        return n;
    }

    kvec_t(rvsdg_node_t*) child_nodes;
    kv_init(child_nodes);
    for (size_t i = 0; i < best->arity; ++i) {
        rvsdg_node_t* c = extract_eclass(g, eg, best->children[i], memo, cost_cache, visiting);
        kv_push(rvsdg_node_t*, child_nodes, c);
    }

    rvsdg_node_t* node = NULL;
    if (best->op == REDVISAGE_EOP_VAR) {
        node = redvisage_make_var(g, "v");
    } else if (best->op == REDVISAGE_EOP_STATE_INIT) {
        node = redvisage_make_state_init(g);
    } else if (best->op == REDVISAGE_EOP_ADD || best->op == REDVISAGE_EOP_MUL ||
               best->op == REDVISAGE_EOP_LT || best->op == REDVISAGE_EOP_PRINT ||
               best->op == REDVISAGE_EOP_YIELD) {
        rvsdg_op_t op = RVSDG_OP_ADD;
        if (best->op == REDVISAGE_EOP_MUL) op = RVSDG_OP_MUL;
        else if (best->op == REDVISAGE_EOP_LT) op = RVSDG_OP_LESS_THAN;
        else if (best->op == REDVISAGE_EOP_PRINT) op = RVSDG_OP_PRINT;
        else if (best->op == REDVISAGE_EOP_YIELD) op = RVSDG_OP_YIELD;
        node = redvisage_make_op(g, op, kv_size(child_nodes), child_nodes.a);
    } else if (best->op == REDVISAGE_EOP_EXTRACT && kv_size(child_nodes) == 1) {
        node = redvisage_make_extract(g, kv_A(child_nodes, 0), 0);
    } else if (best->op == REDVISAGE_EOP_GAMMA && kv_size(child_nodes) >= 3) {
        rvsdg_region_t* t_region = redvisage_region_create(g);
        rvsdg_region_t* f_region = redvisage_region_create(g);
        int64_t has_false = 0;
        rvsdg_node_t* cond = kv_A(child_nodes, 1);
        rvsdg_node_t* t_yield = kv_A(child_nodes, 2);
        redvisage_region_set_yield(t_region, t_yield);
        redvisage_region_add_node(t_region, t_yield);
        extract_const_from_eclass(eg, best->children[0], &has_false);
        if (has_false && kv_size(child_nodes) >= 4) {
            rvsdg_node_t* f_yield = kv_A(child_nodes, 3);
            redvisage_region_set_yield(f_region, f_yield);
            redvisage_region_add_node(f_region, f_yield);
        }
        node = redvisage_make_gamma(g, cond, t_region, f_region);
    } else if (best->op == REDVISAGE_EOP_THETA && kv_size(child_nodes) >= 3) {
        rvsdg_region_t* body = redvisage_region_create(g);
        int64_t carried_count = 0;
        int64_t arg_count = 0;
        size_t i = 0;
        extract_const_from_eclass(eg, best->children[0], &carried_count);
        extract_const_from_eclass(eg, best->children[1], &arg_count);
        if (carried_count < 0) carried_count = 0;
        if (arg_count < 0) arg_count = 0;
        for (i = 0; i < (size_t)arg_count && (2 + (size_t)carried_count + i) < kv_size(child_nodes) - 1; ++i) {
            redvisage_region_add_arg(body, kv_A(child_nodes, 2 + (size_t)carried_count + i));
        }
        redvisage_region_set_yield(body, kv_A(child_nodes, kv_size(child_nodes) - 1));
        redvisage_region_add_node(body, kv_A(child_nodes, kv_size(child_nodes) - 1));
        if ((size_t)carried_count > 0 && (2 + (size_t)carried_count) <= kv_size(child_nodes)) {
            node = redvisage_make_theta(g, body, (size_t)carried_count, &kv_A(child_nodes, 2));
        } else {
            node = redvisage_make_theta(g, body, 0, NULL);
        }
    } else {
        node = redvisage_make_op(g, RVSDG_OP_ADD, kv_size(child_nodes), child_nodes.a);
    }
    kv_destroy(child_nodes);

    int ret;
    it = kh_put(extracted, memo, id, &ret);
    kh_value(memo, it) = node;
    return node;
}

rvsdg_graph_t* redvisage_extract_graph(eqx_egraph_t* egraph, eqx_eclass_id_t root) {
    khash_t(extracted)* memo;
    khash_t(cost_cache)* costs;
    khash_t(visiting)* visiting;
    rvsdg_graph_t* graph = redvisage_graph_create();
    if (!graph || !egraph) return NULL;

    memo = kh_init(extracted);
    costs = kh_init(cost_cache);
    visiting = kh_init(visiting);
    if (!memo || !costs || !visiting) {
        if (memo) kh_destroy(extracted, memo);
        if (costs) kh_destroy(cost_cache, costs);
        if (visiting) kh_destroy(visiting, visiting);
        redvisage_graph_destroy(graph);
        return NULL;
    }

    redvisage_set_root(graph, extract_eclass(graph, egraph, root, memo, costs, visiting));

    kh_destroy(extracted, memo);
    kh_destroy(cost_cache, costs);
    kh_destroy(visiting, visiting);
    return graph;
}
