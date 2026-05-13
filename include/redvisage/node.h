#ifndef REDVISAGE_NODE_H
#define REDVISAGE_NODE_H

#include <stddef.h>
#include <stdint.h>

#include "klib/kvec.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum rvsdg_node_kind {
    RVSDG_NODE_CONST_INT = 0,
    RVSDG_NODE_VAR,
    RVSDG_NODE_OP,
    RVSDG_NODE_GAMMA,
    RVSDG_NODE_THETA,
    RVSDG_NODE_STATE,
    RVSDG_NODE_EXTRACT
} rvsdg_node_kind_t;

typedef enum rvsdg_op {
    RVSDG_OP_ADD = 0,
    RVSDG_OP_MUL,
    RVSDG_OP_LESS_THAN,
    RVSDG_OP_PRINT,
    RVSDG_OP_YIELD
} rvsdg_op_t;

typedef struct rvsdg_node rvsdg_node_t;

typedef struct rvsdg_region {
    uint32_t id;
    kvec_t(rvsdg_node_t*) args;
    kvec_t(rvsdg_node_t*) nodes;
    rvsdg_node_t* yield_node;
} rvsdg_region_t;

struct rvsdg_node {
    uint32_t id;
    rvsdg_node_kind_t kind;
    kvec_t(rvsdg_node_t*) inputs;
    union {
        int64_t const_int;
        const char* var_name;
        rvsdg_op_t op;
        struct {
            rvsdg_node_t* cond;
            rvsdg_region_t* true_region;
            rvsdg_region_t* false_region;
        } gamma;
        struct {
            rvsdg_region_t* body;
        } theta;
        struct {
            rvsdg_node_t* tuple;
            size_t index;
        } extract;
    } data;
};

typedef struct rvsdg_graph {
    uint32_t next_id;
    kvec_t(rvsdg_node_t*) nodes;
    kvec_t(rvsdg_region_t*) regions;
    rvsdg_node_t* root;
} rvsdg_graph_t;

rvsdg_graph_t* redvisage_graph_create(void);
void redvisage_graph_destroy(rvsdg_graph_t* graph);
void redvisage_set_root(rvsdg_graph_t* graph, rvsdg_node_t* root);

rvsdg_region_t* redvisage_region_create(rvsdg_graph_t* graph);
void redvisage_region_add_arg(rvsdg_region_t* region, rvsdg_node_t* arg);
void redvisage_region_add_node(rvsdg_region_t* region, rvsdg_node_t* node);
void redvisage_region_set_yield(rvsdg_region_t* region, rvsdg_node_t* yield_node);

rvsdg_node_t* redvisage_make_const_int(rvsdg_graph_t* graph, int64_t value);
rvsdg_node_t* redvisage_make_var(rvsdg_graph_t* graph, const char* name);
rvsdg_node_t* redvisage_make_state_init(rvsdg_graph_t* graph);
rvsdg_node_t* redvisage_make_op(rvsdg_graph_t* graph, rvsdg_op_t op, size_t input_count, rvsdg_node_t** inputs);
rvsdg_node_t* redvisage_make_theta(rvsdg_graph_t* graph, rvsdg_region_t* body, size_t input_count, rvsdg_node_t** inputs);
rvsdg_node_t* redvisage_make_gamma(rvsdg_graph_t* graph, rvsdg_node_t* cond, rvsdg_region_t* true_region, rvsdg_region_t* false_region);
rvsdg_node_t* redvisage_make_extract(rvsdg_graph_t* graph, rvsdg_node_t* tuple, size_t index);

#ifdef __cplusplus
}
#endif

#endif
