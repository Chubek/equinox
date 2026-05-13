#include "common/dump.h"

#include "redvisage/opcodes.h"

#include <inttypes.h>

static void dump_node(FILE* out, const rvsdg_node_t* node) {
    fprintf(out, "n%u: ", node->id);
    switch (node->kind) {
        case RVSDG_NODE_CONST_INT:
            fprintf(out, "CONST val=%" PRId64 "\n", node->data.const_int);
            break;
        case RVSDG_NODE_VAR:
            fprintf(out, "VAR name=%s\n", node->data.var_name ? node->data.var_name : "_");
            break;
        case RVSDG_NODE_OP:
            fprintf(out, "OP op=%d arity=%zu\n", (int)node->data.op, kv_size(node->inputs));
            break;
        case RVSDG_NODE_GAMMA:
            fprintf(out, "GAMMA cond=n%u\n", node->data.gamma.cond ? node->data.gamma.cond->id : 0);
            break;
        case RVSDG_NODE_THETA:
            fprintf(out, "THETA carried=%zu\n", kv_size(node->inputs));
            break;
        case RVSDG_NODE_STATE:
            fprintf(out, "STATE_INIT\n");
            break;
        case RVSDG_NODE_EXTRACT:
            fprintf(out, "EXTRACT index=%zu\n", node->data.extract.index);
            break;
    }
}

void redvisage_dump_rvsdg(FILE* out, const rvsdg_graph_t* graph) {
    if (!out || !graph) return;
    fprintf(out, "=== REDVISAGE RVSDG DUMP ===\n");
    for (size_t i = 0; i < kv_size(graph->nodes); ++i) {
        const rvsdg_node_t* node = kv_A(graph->nodes, i);
        if (node) dump_node(out, node);
    }
    if (graph->root) {
        fprintf(out, "root=n%u\n", graph->root->id);
    }
    fprintf(out, "============================\n");
}

void redvisage_dump_egraph(FILE* out, eqx_egraph_t* egraph) {
    if (!out || !egraph) return;
    eqx_egraph_stats_t st;
    eqx_egraph_get_stats(egraph, &st);
    fprintf(out, "=== EQUINOX E-GRAPH DUMP ===\n");
    fprintf(out, "[E-Graph Size: %zu classes, %zu nodes]\n", st.num_eclasses, st.num_enodes);

    eqx_egraph_iter_t it = eqx_egraph_iter_begin(egraph);
    while (eqx_egraph_iter_has_next(it)) {
        eqx_eclass_id_t id = eqx_egraph_iter_next(it);
        eqx_eclass_t* ec = eqx_egraph_get_eclass(egraph, id);
        fprintf(out, "e%u : { ", id);
        if (ec) {
            for (eqx_enode_t* n = ec->nodes; n != NULL; n = n->next_in_class) {
                if (redvisage_is_const_opcode(n->op)) {
                    fprintf(out, "(CONST %" PRId64 ") ", redvisage_decode_const_int(n->op));
                } else {
                    fprintf(out, "(op=%u arity=%zu) ", n->op, n->arity);
                }
            }
        }
        fprintf(out, "}\n");
    }
    eqx_egraph_iter_end(it);
    fprintf(out, "============================\n");
}
