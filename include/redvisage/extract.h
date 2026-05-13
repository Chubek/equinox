#ifndef REDVISAGE_EXTRACT_H
#define REDVISAGE_EXTRACT_H

#include "equinox/egraph.h"
#include "redvisage/node.h"

#ifdef __cplusplus
extern "C" {
#endif

rvsdg_graph_t* redvisage_extract_graph(eqx_egraph_t* egraph, eqx_eclass_id_t root);

#ifdef __cplusplus
}
#endif

#endif
