#ifndef EQX_COMMON_EQVSG_H
#define EQX_COMMON_EQVSG_H

#include <stddef.h>

#include "equinox/egraph.h"
#include "redvisage/node.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redvisage_eqvsg_options {
    const char* input_file;
    const char* output_file;
    int opt_level;
    int verbose;
    int dump_rvsdg;
    int dump_egraph;
    const char* emit_dot_dir;
    int run;
    int argc;
    const char** argv;
    size_t rules_file_count;
    const char** rules_files;
} redvisage_eqvsg_options_t;

redvisage_eqvsg_options_t redvisage_eqvsg_options_default(void);
int redvisage_eqvsg_parse_file(const char* path, rvsdg_graph_t** out_graph);
int redvisage_eqvsg_run_pipeline(const redvisage_eqvsg_options_t* options);
int redvisage_eqvsg_main(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif
