/**
 * @file Manual.i
 * @brief Main Doxygen documentation entry for Equinox and RedVisage
 */

/**
 * @mainpage Equinox + RedVisage Manual
 *
 * @section intro_sec Introduction
 *
 * Equinox is a C library for equality saturation based on e-graphs.
 * RedVisage is the RVSDG frontend layered on top of Equinox, with
 * explicit state-token threading and structured control-flow nodes.
 *
 * Major layers in this repository:
 * - **Equinox Core**: e-graph, e-class/e-node storage, union-find, hashcons, rewrites
 * - **RedVisage**: RVSDG IR, lowering into Equinox, RVSDG-oriented rewrites, extraction
 * - **EQVSG (planned/common layer)**: S-expression language and CLI pipeline
 *
 * @section features_sec Key Features
 *
 * - Efficient e-graph implementation with union-find canonicalization
 * - Hash-consing for structural sharing of enodes
 * - Congruence closure via rebuild
 * - Pattern-based rewrite engine (`eqx_pattern_*`, `eqx_rewrite_*`)
 * - RedVisage RVSDG nodes: pure ops, gamma/theta, state tokens, extract
 * - RedVisage lowering with memoization (`khash`) and explicit child threading
 * - RedVisage extraction with memoization and structured reconstruction
 *
 * @section arch_sec Architecture
 *
 * @subsection arch_core Equinox Core
 * - @ref enode.h: `eqx_enode_t`
 * - @ref eclass.h: `eqx_eclass_t`
 * - @ref egraph.h: `eqx_egraph_t` and iteration/statistics APIs
 * - @ref unionfind.h: canonical representative management
 * - @ref hashcons.h: structural deduplication
 * - @ref rewrite.h: pattern matching and rewrite execution
 *
 * @subsection arch_redvisage RedVisage Layer
 * - `include/redvisage/node.h`: RVSDG graph, region, and node definitions
 * - `include/redvisage/lower.h`: `redvisage_lower_to_egraph(...)`
 * - `include/redvisage/rewrite.h`: default RedVisage rewrite set
 * - `include/redvisage/extract.h`: `redvisage_extract_graph(...)`
 * - `include/redvisage/opcodes.h`: stable opcode/metadata encoding used by lowering/extraction
 *
 * @subsection arch_common Common/Language Direction
 * The `src/common` and `include/common` trees are intended for EQVSG language,
 * dumping, rule language (ERL), memory wrappers, and compiler-profile tooling.
 * This is the integration surface for `eqvsg` CLI functionality.
 *
 * @section quickstart_sec Equinox Quick Start
 *
 * @subsection quickstart_create Create Graph
 * @code{.c}
 * #include "equinox/egraph.h"
 *
 * eqx_egraph_config_t cfg = eqx_egraph_config_default();
 * eqx_egraph_t* eg = eqx_egraph_create(&cfg);
 * @endcode
 *
 * @subsection quickstart_add Add Terms
 * @code{.c}
 * enum { OP_A = 1, OP_B = 2, OP_F = 10 };
 *
 * eqx_eclass_id_t a = eqx_egraph_add(eg, OP_A, 0, NULL);
 * eqx_eclass_id_t b = eqx_egraph_add(eg, OP_B, 0, NULL);
 * eqx_eclass_id_t ch[] = { a, b };
 * eqx_eclass_id_t f_ab = eqx_egraph_add(eg, OP_F, 2, ch);
 * @endcode
 *
 * @subsection quickstart_union Rewrite and Rebuild
 * @code{.c}
 * eqx_egraph_union(eg, a, b);
 * eqx_egraph_rebuild(eg);
 * @endcode
 *
 * @subsection quickstart_rule Pattern Rule
 * @code{.c}
 * enum { OP_ADD = 20, OP_ZERO = 21 };
 *
 * eqx_pattern_t* x = eqx_pattern_var("x");
 * eqx_pattern_t* z = eqx_pattern_app(OP_ZERO, NULL, 0);
 * eqx_pattern_t* lhs_children[] = { x, z };
 * eqx_pattern_t* lhs = eqx_pattern_app(OP_ADD, lhs_children, 2);
 * eqx_pattern_t* rhs = eqx_pattern_var("x");
 *
 * eqx_rewrite_rule_t* add_zero = eqx_rewrite_rule_create("add-zero", lhs, rhs, NULL);
 * eqx_rewrite_apply_all(add_zero, eg);
 * eqx_egraph_rebuild(eg);
 * eqx_rewrite_rule_destroy(add_zero);
 * @endcode
 *
 * @subsection quickstart_destroy Cleanup
 * @code{.c}
 * eqx_egraph_destroy(eg);
 * @endcode
 *
 * @section redvisage_sec RedVisage Usage
 *
 * RedVisage models data flow and side effects explicitly. State is represented as
 * values in the graph (state tokens) and must be threaded through side-effecting ops.
 *
 * @subsection redvisage_nodes RVSDG Node Kinds
 * - `RVSDG_NODE_CONST_INT`
 * - `RVSDG_NODE_VAR`
 * - `RVSDG_NODE_OP` (`RVSDG_OP_ADD`, `RVSDG_OP_MUL`, `RVSDG_OP_LESS_THAN`, `RVSDG_OP_PRINT`, `RVSDG_OP_YIELD`)
 * - `RVSDG_NODE_GAMMA` (branch)
 * - `RVSDG_NODE_THETA` (loop)
 * - `RVSDG_NODE_STATE`
 * - `RVSDG_NODE_EXTRACT`
 *
 * @subsection redvisage_pipeline RVSDG -> E-Graph -> RVSDG
 * @code{.c}
 * #include "equinox/egraph.h"
 * #include "redvisage/node.h"
 * #include "redvisage/lower.h"
 * #include "redvisage/rewrite.h"
 * #include "redvisage/extract.h"
 *
 * rvsdg_graph_t* g = redvisage_graph_create();
 * rvsdg_node_t* x = redvisage_make_var(g, "x");
 * rvsdg_node_t* two = redvisage_make_const_int(g, 2);
 * rvsdg_node_t* mul_inputs[] = { x, two };
 * rvsdg_node_t* m = redvisage_make_op(g, RVSDG_OP_MUL, 2, mul_inputs);
 * redvisage_set_root(g, m);
 *
 * eqx_egraph_t* eg = eqx_egraph_create(NULL);
 * eqx_eclass_id_t root = redvisage_lower_to_egraph(g, eg);
 *
 * size_t n_rules = 0;
 * eqx_rewrite_rule_t** rules = redvisage_default_rules(&n_rules);
 * eqx_rewrite_apply_rules(rules, n_rules, eg, 8);
 *
 * rvsdg_graph_t* opt = redvisage_extract_graph(eg, root);
 *
 * redvisage_default_rules_destroy(rules, n_rules);
 * redvisage_graph_destroy(opt);
 * eqx_egraph_destroy(eg);
 * redvisage_graph_destroy(g);
 * @endcode
 *
 * @subsection redvisage_regions Regions, Gamma, Theta
 * Regions (`rvsdg_region_t`) own argument lists and a yield node.
 * Current lowering encodes gamma/theta interface metadata directly into enode
 * children so extraction can reconstruct variable branch/loop arities.
 *
 * @section eqvsg_sec EQVSG Language and CLI (Design Intent)
 *
 * EQVSG is the S-expression front-end intended to map source IL to RVSDG,
 * run equality saturation, and emit optimized RVSDG or downstream artifacts.
 *
 * Planned CLI shape:
 * - `eqvsg [OPTIONS] <INPUT_FILE>`
 * - Optimization levels, custom rules, dump controls, dot emission, run mode
 *
 * Related subsystems are expected under:
 * - `src/common/eqvsg` + `include/common/eqvsg.h`
 * - `src/common/dump` + `include/common/dump.h`
 * - `src/common/erl` + `include/common/erl.h`
 * - `src/common/compiler` + `include/common/compiler.h`
 *
 * @section build_sec Building
 *
 * @subsection build_cmake CMake
 * @code{.sh}
 * cmake -S . -B build
 * cmake --build build
 * @endcode
 *
 * Main CMake options:
 * - `EQX_BUILD_TESTS` (legacy alias for `EQX_BUILD_UNITTEST`)
 * - `EQX_BUILD_UNITTEST`
 * - `EQX_BUILD_FUZZ`
 * - `EQX_BUILD_DOCS`
 * - `EQX_BUILD_SHARED`
 * - `EQX_BUILD_STATIC`
 *
 * @subsection build_meson Meson
 * @code{.sh}
 * meson setup build-meson
 * meson compile -C build-meson
 * @endcode
 *
 * @section test_sec Testing
 *
 * @code{.sh}
 * ctest --test-dir build --output-on-failure
 * @endcode
 *
 * RedVisage integration coverage currently includes end-to-end tests in
 * `tests/test_redvisage.c` (build RVSDG, lower, rewrite, extract).
 *
 * @section perf_sec Performance Notes
 *
 * - Equinox core operations remain amortized-efficient due to hashcons + union-find
 * - Saturation cost is rewrite-set and iteration-limit dependent
 * - RedVisage lowering/extraction use memoized traversals (`khash`) to avoid duplication
 *
 * @section thread_sec Thread Safety
 *
 * Equinox and RedVisage APIs are not internally synchronized.
 * Use one graph per thread or external synchronization.
 *
 * @section license_sec License
 *
 * MIT License (see `LICENSE`).
 */

/** @defgroup core Core Components */
/** @defgroup rewrite Rewrite System */
/** @defgroup redvisage RedVisage RVSDG Frontend */
/** @defgroup common Common / EQVSG Layers */

/**
 * @page faq FAQ
 *
 * @section faq_egraph What is an e-graph?
 * A compact representation of many equivalent terms, organized by e-classes.
 *
 * @section faq_redvisage What does RedVisage add?
 * Structured RVSDG nodes (gamma/theta), explicit state-token flow, and
 * lowering/extraction bridges to Equinox.
 *
 * @section faq_threadsafe Is it thread-safe?
 * No. Use external synchronization for shared access.
 */

/**
 * @page changelog Changelog
 *
 * @section v0_1_0 Version 0.1.0
 * - Equinox core e-graph, rewrite engine, and tests.
 *
 * @section v0_2_0_work Version 0.2.0 (in progress)
 * - RedVisage RVSDG node/region layer.
 * - RVSDG lowering and extraction bridges.
 * - RedVisage rewrite scaffolding and tests.
 * - Build system integration for RedVisage sources.
 */

/**
 * @page eqvsg_cli EQVSG CLI Reference
 *
 * @section eqvsg_synopsis Synopsis
 *
 * @code{.text}
 * eqvsg [OPTIONS] <INPUT_FILE>
 * @endcode
 *
 * @section eqvsg_args Arguments
 *
 * - `<INPUT_FILE>`: Path to input IL file (`.vsg` or `.il`) containing S-expressions.
 *
 * @section eqvsg_options Options
 *
 * - `-o, --output <FILE>`
 *   - Write optimized RVSDG to `<FILE>`. If omitted, output goes to stdout.
 *
 * - `-O, --opt-level <LEVEL>`
 *   - Saturation configuration profile:
 *     - `0`: Parse/lower only (no optimization)
 *     - `1`: Basic rules, node limit 10,000 (default)
 *     - `2`: Aggressive rules, loop-oriented transforms, node limit 50,000
 *
 * - `-r, --rules <FILE>`
 *   - Path to custom rewrite ruleset (`.rules`).
 *   - Overrides built-in rules for selected optimization level.
 *
 * @section eqvsg_debug Debug and Dumping
 *
 * - `--dump-rvsdg`
 *   - Dump parsed RedVisage RVSDG to stderr before lowering.
 *
 * - `--dump-egraph`
 *   - Dump Equinox e-graph classes after saturation.
 *
 * - `--emit-dot <DIR>`
 *   - Emit Graphviz `.dot` files for pipeline stages into `<DIR>`.
 *
 * @section eqvsg_exec Execution
 *
 * - `--run`
 *   - Execute/interpet extracted optimized RVSDG immediately (when runtime is enabled).
 *
 * - `-a, --args <ARGS>...`
 *   - Positional arguments passed to runtime execution mode.
 *
 * @section eqvsg_general General
 *
 * - `-v, --verbose`: Enable verbose logs (iteration/saturation stats).
 * - `-h, --help`: Show help.
 * - `-V, --version`: Show version.
 *
 * @section eqvsg_examples Examples
 *
 * @code{.sh}
 * # Parse + optimize with defaults
 * eqvsg input.vsg
 *
 * # Aggressive optimization and custom output
 * eqvsg -O 2 -o out.vsg input.vsg
 *
 * # Use custom rewrite file and emit debug graphs
 * eqvsg --rules rules/basic.rules --dump-rvsdg --dump-egraph --emit-dot ./dot input.il
 *
 * # Run optimized program with arguments
 * eqvsg --run -a foo bar baz input.vsg
 * @endcode
 */

#endif /* EQUINOX_DOXYGEN_I */
