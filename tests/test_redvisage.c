#include <catch2/catch_test_macros.hpp>

#include "equinox/egraph.h"
#include "equinox/rewrite.h"
#include "redvisage/node.h"
#include "redvisage/lower.h"
#include "redvisage/extract.h"
#include "redvisage/rewrite.h"

TEST_CASE("redvisage lower + rewrite + extract", "[redvisage]") {
    rvsdg_graph_t* graph = redvisage_graph_create();
    REQUIRE(graph != nullptr);

    rvsdg_node_t* x = redvisage_make_var(graph, "x");
    rvsdg_node_t* mul_inputs[] = {x, x};
    rvsdg_node_t* mul = redvisage_make_op(graph, RVSDG_OP_MUL, 2, mul_inputs);
    redvisage_set_root(graph, mul);

    eqx_egraph_config_t cfg = eqx_egraph_config_default();
    eqx_egraph_t* eg = eqx_egraph_create(&cfg);
    REQUIRE(eg != nullptr);

    eqx_eclass_id_t root = redvisage_lower_to_egraph(graph, eg);
    REQUIRE(root != EQX_ECLASS_ID_INVALID);

    size_t rule_count = 0;
    eqx_rewrite_rule_t** rules = redvisage_default_rules(&rule_count);
    REQUIRE(rules != nullptr);
    REQUIRE(rule_count > 0);

    size_t changed = eqx_rewrite_apply_rules(rules, rule_count, eg, 2);
    REQUIRE(changed >= 1);

    rvsdg_graph_t* extracted = redvisage_extract_graph(eg, root);
    REQUIRE(extracted != nullptr);
    REQUIRE(extracted->root != nullptr);
    REQUIRE(extracted->root->kind == RVSDG_NODE_OP);
    REQUIRE(extracted->root->data.op == RVSDG_OP_ADD);

    redvisage_graph_destroy(extracted);
    redvisage_default_rules_destroy(rules, rule_count);
    eqx_egraph_destroy(eg);
    redvisage_graph_destroy(graph);
}

TEST_CASE("redvisage theta lowering carries loop interface", "[redvisage]") {
    rvsdg_graph_t* graph = redvisage_graph_create();
    REQUIRE(graph != nullptr);

    rvsdg_node_t* s0 = redvisage_make_state_init(graph);
    rvsdg_node_t* i0 = redvisage_make_const_int(graph, 0);
    rvsdg_node_t* one = redvisage_make_const_int(graph, 1);

    rvsdg_region_t* body = redvisage_region_create(graph);
    rvsdg_node_t* s_in = redvisage_make_var(graph, "s_in");
    rvsdg_node_t* i_in = redvisage_make_var(graph, "i_in");
    redvisage_region_add_arg(body, s_in);
    redvisage_region_add_arg(body, i_in);

    rvsdg_node_t* print_inputs[] = {s_in, i_in};
    rvsdg_node_t* s_next = redvisage_make_op(graph, RVSDG_OP_PRINT, 2, print_inputs);
    rvsdg_node_t* add_inputs[] = {i_in, one};
    rvsdg_node_t* i_next = redvisage_make_op(graph, RVSDG_OP_ADD, 2, add_inputs);
    rvsdg_node_t* yield_inputs[] = {s_next, i_next};
    rvsdg_node_t* y = redvisage_make_op(graph, RVSDG_OP_YIELD, 2, yield_inputs);
    redvisage_region_add_node(body, s_next);
    redvisage_region_add_node(body, i_next);
    redvisage_region_set_yield(body, y);

    rvsdg_node_t* theta_inputs[] = {s0, i0};
    rvsdg_node_t* theta = redvisage_make_theta(graph, body, 2, theta_inputs);
    redvisage_set_root(graph, theta);

    eqx_egraph_t* eg = eqx_egraph_create(nullptr);
    REQUIRE(eg != nullptr);
    eqx_eclass_id_t root = redvisage_lower_to_egraph(graph, eg);
    REQUIRE(root != EQX_ECLASS_ID_INVALID);

    eqx_eclass_t* ec = eqx_egraph_get_eclass(eg, root);
    REQUIRE(ec != nullptr);
    REQUIRE(ec->nodes != nullptr);
    REQUIRE(ec->nodes->arity >= 5);

    rvsdg_graph_t* extracted = redvisage_extract_graph(eg, root);
    REQUIRE(extracted != nullptr);
    REQUIRE(extracted->root != nullptr);
    REQUIRE(extracted->root->kind == RVSDG_NODE_THETA);
    REQUIRE(extracted->root->data.theta.body != nullptr);
    REQUIRE(extracted->root->data.theta.body->yield_node != nullptr);
    REQUIRE(kv_size(extracted->root->inputs) == 2);

    redvisage_graph_destroy(extracted);
    eqx_egraph_destroy(eg);
    redvisage_graph_destroy(graph);
}

TEST_CASE("redvisage theta extraction preserves variable carried arity", "[redvisage]") {
    rvsdg_graph_t* graph = redvisage_graph_create();
    REQUIRE(graph != nullptr);
    rvsdg_node_t* s0 = redvisage_make_state_init(graph);
    rvsdg_node_t* i0 = redvisage_make_const_int(graph, 0);
    rvsdg_node_t* a0 = redvisage_make_const_int(graph, 7);
    rvsdg_region_t* body = redvisage_region_create(graph);
    rvsdg_node_t* y = redvisage_make_var(graph, "y");
    redvisage_region_set_yield(body, y);
    rvsdg_node_t* theta_inputs[] = {s0, i0, a0};
    rvsdg_node_t* theta = redvisage_make_theta(graph, body, 3, theta_inputs);
    redvisage_set_root(graph, theta);

    eqx_egraph_t* eg = eqx_egraph_create(nullptr);
    REQUIRE(eg != nullptr);
    eqx_eclass_id_t root = redvisage_lower_to_egraph(graph, eg);
    REQUIRE(root != EQX_ECLASS_ID_INVALID);

    rvsdg_graph_t* extracted = redvisage_extract_graph(eg, root);
    REQUIRE(extracted != nullptr);
    REQUIRE(extracted->root != nullptr);
    REQUIRE(extracted->root->kind == RVSDG_NODE_THETA);
    REQUIRE(kv_size(extracted->root->inputs) == 3);

    redvisage_graph_destroy(extracted);
    eqx_egraph_destroy(eg);
    redvisage_graph_destroy(graph);
}
