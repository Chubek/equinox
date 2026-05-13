#include "redvisage/node.h"

#include <stdlib.h>
#include <string.h>

static rvsdg_node_t* redvisage_alloc_node(rvsdg_graph_t* graph, rvsdg_node_kind_t kind) {
    if (!graph) {
        return NULL;
    }
    rvsdg_node_t* node = (rvsdg_node_t*)calloc(1, sizeof(*node));
    if (!node) {
        return NULL;
    }
    node->id = graph->next_id++;
    node->kind = kind;
    kv_init(node->inputs);
    kv_push(rvsdg_node_t*, graph->nodes, node);
    return node;
}

rvsdg_graph_t* redvisage_graph_create(void) {
    rvsdg_graph_t* graph = (rvsdg_graph_t*)calloc(1, sizeof(*graph));
    if (!graph) {
        return NULL;
    }
    graph->next_id = 1;
    kv_init(graph->nodes);
    kv_init(graph->regions);
    graph->root = NULL;
    return graph;
}

void redvisage_graph_destroy(rvsdg_graph_t* graph) {
    size_t i;
    if (!graph) {
        return;
    }

    for (i = 0; i < kv_size(graph->nodes); ++i) {
        free(kv_A(graph->nodes, i));
    }
    kv_destroy(graph->nodes);

    for (i = 0; i < kv_size(graph->regions); ++i) {
        rvsdg_region_t* region = kv_A(graph->regions, i);
        if (!region) {
            continue;
        }
        kv_destroy(region->args);
        kv_destroy(region->nodes);
        free(region);
    }
    kv_destroy(graph->regions);

    free(graph);
}

void redvisage_set_root(rvsdg_graph_t* graph, rvsdg_node_t* root) {
    if (graph) {
        graph->root = root;
    }
}

rvsdg_region_t* redvisage_region_create(rvsdg_graph_t* graph) {
    rvsdg_region_t* region;
    if (!graph) {
        return NULL;
    }
    region = (rvsdg_region_t*)calloc(1, sizeof(*region));
    if (!region) {
        return NULL;
    }
    region->id = graph->next_id++;
    kv_init(region->args);
    kv_init(region->nodes);
    kv_push(rvsdg_region_t*, graph->regions, region);
    return region;
}

void redvisage_region_add_arg(rvsdg_region_t* region, rvsdg_node_t* arg) {
    if (region && arg) {
        kv_push(rvsdg_node_t*, region->args, arg);
    }
}

void redvisage_region_add_node(rvsdg_region_t* region, rvsdg_node_t* node) {
    if (region && node) {
        kv_push(rvsdg_node_t*, region->nodes, node);
    }
}

void redvisage_region_set_yield(rvsdg_region_t* region, rvsdg_node_t* yield_node) {
    if (region) {
        region->yield_node = yield_node;
    }
}

rvsdg_node_t* redvisage_make_const_int(rvsdg_graph_t* graph, int64_t value) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_CONST_INT);
    if (node) {
        node->data.const_int = value;
    }
    return node;
}

rvsdg_node_t* redvisage_make_var(rvsdg_graph_t* graph, const char* name) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_VAR);
    if (node) {
        node->data.var_name = name;
    }
    return node;
}

rvsdg_node_t* redvisage_make_state_init(rvsdg_graph_t* graph) {
    return redvisage_alloc_node(graph, RVSDG_NODE_STATE);
}

rvsdg_node_t* redvisage_make_op(rvsdg_graph_t* graph, rvsdg_op_t op, size_t input_count, rvsdg_node_t** inputs) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_OP);
    size_t i;
    if (!node) {
        return NULL;
    }
    node->data.op = op;
    for (i = 0; i < input_count; ++i) {
        kv_push(rvsdg_node_t*, node->inputs, inputs[i]);
    }
    return node;
}

rvsdg_node_t* redvisage_make_theta(rvsdg_graph_t* graph, rvsdg_region_t* body, size_t input_count, rvsdg_node_t** inputs) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_THETA);
    size_t i;
    if (!node) {
        return NULL;
    }
    node->data.theta.body = body;
    for (i = 0; i < input_count; ++i) {
        kv_push(rvsdg_node_t*, node->inputs, inputs[i]);
    }
    return node;
}

rvsdg_node_t* redvisage_make_gamma(rvsdg_graph_t* graph, rvsdg_node_t* cond, rvsdg_region_t* true_region, rvsdg_region_t* false_region) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_GAMMA);
    if (!node) {
        return NULL;
    }
    node->data.gamma.cond = cond;
    node->data.gamma.true_region = true_region;
    node->data.gamma.false_region = false_region;
    kv_push(rvsdg_node_t*, node->inputs, cond);
    return node;
}

rvsdg_node_t* redvisage_make_extract(rvsdg_graph_t* graph, rvsdg_node_t* tuple, size_t index) {
    rvsdg_node_t* node = redvisage_alloc_node(graph, RVSDG_NODE_EXTRACT);
    if (!node) {
        return NULL;
    }
    node->data.extract.tuple = tuple;
    node->data.extract.index = index;
    kv_push(rvsdg_node_t*, node->inputs, tuple);
    return node;
}
