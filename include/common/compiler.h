#ifndef EQX_COMMON_COMPILER_H
#define EQX_COMMON_COMPILER_H

#include "redvisage/node.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redvisage_compile_options {
    const char* profile_path;
    const char* output_path;
    int emit_comments;
} redvisage_compile_options_t;

int redvisage_compile_to_asm(const rvsdg_graph_t* graph, const redvisage_compile_options_t* options);

#ifdef __cplusplus
}
#endif

#endif
