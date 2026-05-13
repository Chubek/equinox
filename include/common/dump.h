#ifndef EQX_COMMON_DUMP_H
#define EQX_COMMON_DUMP_H

#include <stdio.h>

#include "equinox/egraph.h"
#include "redvisage/node.h"

#ifdef __cplusplus
extern "C" {
#endif

void redvisage_dump_rvsdg(FILE* out, const rvsdg_graph_t* graph);
void redvisage_dump_egraph(FILE* out, eqx_egraph_t* egraph);

#ifdef __cplusplus
}
#endif

#endif
