#include "common/eqvsg.h"

#include "common/dump.h"
#include "common/erl.h"
#include "redvisage/extract.h"
#include "redvisage/lower.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(FILE* out) {
    fprintf(out,
        "Usage: eqvsg [OPTIONS] <INPUT_FILE>\n"
        "\n"
        "Options:\n"
        "  -o, --output <FILE>\n"
        "  -O, --opt-level <LEVEL>\n"
        "  -r, --rules <FILE>\n"
        "  --dump-rvsdg\n"
        "  --dump-egraph\n"
        "  --emit-dot <DIR>\n"
        "  --run\n"
        "  -a, --args <ARGS>...\n"
        "  -v, --verbose\n"
        "  -h, --help\n"
        "  -V, --version\n");
}

redvisage_eqvsg_options_t redvisage_eqvsg_options_default(void) {
    redvisage_eqvsg_options_t opt;
    memset(&opt, 0, sizeof(opt));
    opt.opt_level = 1;
    return opt;
}

int redvisage_eqvsg_parse_file(const char* path, rvsdg_graph_t** out_graph) {
    FILE* f;
    rvsdg_graph_t* g;
    rvsdg_node_t* c0;
    if (!path || !out_graph) return -1;

    f = fopen(path, "r");
    if (!f) return -1;
    fclose(f);

    /* Minimal parser scaffold: produce a graph shell to drive the pipeline. */
    g = redvisage_graph_create();
    if (!g) return -1;
    c0 = redvisage_make_const_int(g, 0);
    redvisage_set_root(g, c0);
    *out_graph = g;
    return 0;
}

static int parse_cli(int argc, char** argv, redvisage_eqvsg_options_t* opt) {
    int i;
    if (!opt) return -1;

    *opt = redvisage_eqvsg_options_default();
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(stdout);
            return 1;
        }
        if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--version")) {
            fprintf(stdout, "eqvsg 1.0.0\n");
            return 1;
        }
        if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) && i + 1 < argc) {
            opt->output_file = argv[++i];
            continue;
        }
        if ((!strcmp(argv[i], "-O") || !strcmp(argv[i], "--opt-level")) && i + 1 < argc) {
            opt->opt_level = atoi(argv[++i]);
            continue;
        }
        if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--rules")) {
            if (i + 1 >= argc) return -1;
            i++;
            opt->rules_file_count += 1;
            opt->rules_files = (const char**)realloc((void*)opt->rules_files,
                opt->rules_file_count * sizeof(*opt->rules_files));
            opt->rules_files[opt->rules_file_count - 1] = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "--dump-rvsdg")) {
            opt->dump_rvsdg = 1;
            continue;
        }
        if (!strcmp(argv[i], "--dump-egraph")) {
            opt->dump_egraph = 1;
            continue;
        }
        if (!strcmp(argv[i], "--emit-dot") && i + 1 < argc) {
            opt->emit_dot_dir = argv[++i];
            continue;
        }
        if (!strcmp(argv[i], "--run")) {
            opt->run = 1;
            continue;
        }
        if ((!strcmp(argv[i], "-a") || !strcmp(argv[i], "--args")) && i + 1 < argc) {
            opt->argv = (const char**)&argv[i + 1];
            opt->argc = argc - (i + 1);
            break;
        }
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            opt->verbose = 1;
            continue;
        }
        if (argv[i][0] == '-') {
            return -1;
        }
        opt->input_file = argv[i];
    }

    return opt->input_file ? 0 : -1;
}

int redvisage_eqvsg_run_pipeline(const redvisage_eqvsg_options_t* options) {
    rvsdg_graph_t* parsed = NULL;
    rvsdg_graph_t* extracted = NULL;
    redvisage_erl_ruleset_t rules = {0};
    eqx_egraph_t* eg = NULL;
    eqx_eclass_id_t root_id;
    eqx_egraph_config_t cfg;
    FILE* out = stdout;

    if (!options || !options->input_file) return -1;
    if (redvisage_eqvsg_parse_file(options->input_file, &parsed) != 0) return -1;

    if (options->dump_rvsdg) {
        redvisage_dump_rvsdg(stderr, parsed);
    }

    cfg = eqx_egraph_config_default();
    cfg.node_limit = options->opt_level >= 2 ? 50000 : 10000;
    eg = eqx_egraph_create(&cfg);
    if (!eg) goto fail;

    root_id = redvisage_lower_to_egraph(parsed, eg);
    if (root_id == EQX_ECLASS_ID_INVALID) goto fail;

    if (redvisage_erl_load_files(options->rules_file_count, options->rules_files, &rules) != 0) goto fail;
    if (options->opt_level > 0) {
        (void)eqx_rewrite_apply_rules(rules.rules, rules.count, eg, options->opt_level >= 2 ? 8 : 4);
    }

    if (options->dump_egraph) {
        redvisage_dump_egraph(stderr, eg);
    }

    extracted = redvisage_extract_graph(eg, root_id);
    if (!extracted) goto fail;

    if (options->output_file) {
        out = fopen(options->output_file, "w");
        if (!out) goto fail;
    }
    fprintf(out, ";; EQVSG output root kind=%d\n", extracted->root ? (int)extracted->root->kind : -1);
    if (out != stdout) fclose(out);

    redvisage_erl_ruleset_destroy(&rules);
    eqx_egraph_destroy(eg);
    redvisage_graph_destroy(extracted);
    redvisage_graph_destroy(parsed);
    free((void*)options->rules_files);
    return 0;

fail:
    if (out != stdout) fclose(out);
    redvisage_erl_ruleset_destroy(&rules);
    if (eg) eqx_egraph_destroy(eg);
    if (extracted) redvisage_graph_destroy(extracted);
    if (parsed) redvisage_graph_destroy(parsed);
    free((void*)options->rules_files);
    return -1;
}

int redvisage_eqvsg_main(int argc, char** argv) {
    redvisage_eqvsg_options_t opt;
    int rc = parse_cli(argc, argv, &opt);
    if (rc == 1) return 0;
    if (rc != 0) {
        print_usage(stderr);
        return 2;
    }
    return redvisage_eqvsg_run_pipeline(&opt) == 0 ? 0 : 1;
}
