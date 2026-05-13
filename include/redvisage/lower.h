#ifndef REDVISAGE_LOWER_H
#define REDVISAGE_LOWER_H

#include "equinox/egraph.h"
#include "redvisage/node.h"

#ifdef __cplusplus
extern "C" {
#endif

eqx_eclass_id_t redvisage_lower_to_egraph(rvsdg_graph_t* graph, eqx_egraph_t* egraph);

#ifdef __cplusplus
}
#endif

#endif
