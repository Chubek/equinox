# AGENTS.md: RedVisage Implementation Guide

## Objective
Implement "RedVisage", a Regional Value State Dependence Graph (RVSDG) frontend for the "Equinox" E-Graph library in C. Equinox is the core equality saturation engine, while RedVisage represents programs as hierarchical, acyclic graphs using $\gamma$ (branch) and $\theta$ (loop) nodes, explicitly threading state tokens.

## General Guidelines
- Do not modify the core `equinox` files.
- Write modern, memory-safe C code. 
- Prefix all RedVisage functions and structs with `redvisage_` or `rvsdg_`.
- **Mandatory Data Structures:** You must use `klib` (located in the `third_party/` directory) for all dynamic arrays and hash maps. Specifically, use `kvec.h` for node children/lists and `khash.h` for memoization and ID mapping.

## Implementation Steps

### 1. Define the IR (`node.h` & `node.c`)
- Define the base `rvsdg_node` struct. Use `kvec_t` from `kvec.h` to store dynamic lists of children/arguments.
- Implement specialized node types:
  - **Pure Operations:** Simple math/logic (e.g., $add$, $mul$).
  - **$\gamma$ (Gamma) Nodes:** For branching/conditionals.
  - **$\theta$ (Theta) Nodes:** For loops, carrying state and values across iterations.
  - **State Tokens:** Explicit inputs/outputs for side-effecting operations to model memory dependencies.
- Implement constructors and memory management for these nodes.

### 2. Lowering to Equinox (`lower.h` & `lower.c`)
- Write a traversal function `redvisage_lower_to_egraph(rvsdg_graph* g, egraph* eg)`.
- Use `khash.h` to create a map between `rvsdg_node*` (or their IDs) and Equinox `eclass_id`s to handle shared subgraphs and avoid infinite recursion.
- Translate each `rvsdg_node` into an Equinox `enode`.
- **Crucial:** Ensure state tokens are passed as explicit children to `enode`s so the E-Graph correctly models memory flow.

### 3. RVSDG-Specific Rewrites (`rewrite.h` & `rewrite.c`)
- Define rewrite rules specific to RVSDG semantics.
- Implement rules for:
  - Loop invariant code motion (extracting pure ops out of $\theta$ nodes).
  - Branch folding/simplification on $\gamma$ nodes.
- Register these rules with Equinox's core `rewrite.c` engine.

### 4. Extraction (`extract.h` & `extract.c`)
- Implement a cost function evaluating Equinox E-Classes.
- Write `redvisage_extract_graph(egraph* eg, eclass_id root)` to walk the saturated E-Graph and rebuild a fresh, optimized `rvsdg_graph` by picking the lowest-cost nodes.
- Use `khash.h` to keep track of already extracted E-Classes to prevent duplication in the reconstructed RVSDG.

### 5. Update Build Systems
- **CMakeLists.txt:** 
  - Add the new `src/redvisage/*.c` files to the main library target. Add `include/` to the include directories.
  - **Klib Integration:** Include the custom CMake handler by adding `include(cmake/HandleKlib.cmake)`. Ensure that whatever `klib` headers are imported, the build system pulls them from the `third_party/` directory using this CMake script.
  - Implement rest of the files in `cmake` directory, especially `cmake/equinox.pc.in`.
- **meson.build:** Update the source arrays to include the RedVisage C files, and configure the include directories to point to `third_party/` for `klib` headers.

### 6. Tests (`tests/`)
- Create `tests/test_redvisage.c`.
- Write tests that:
  1. Build a simple RVSDG (e.g., a loop $I = \theta(0, I + 1)$).
  2. Lower it to Equinox.
  3. Apply a dummy rewrite rule.
  4. Extract the graph and assert the structure has changed.
- Integrate the test into the CTest (CMake) and Meson test suites.

## Usage Example

This is an example of using RedVisage alongside Equinox:

```c
#include <stdio.h>
#include "equinox/egraph.h"
#include "redvisage/node.h"
#include "redvisage/lower.h"
#include "redvisage/rewrite.h"
#include "redvisage/extract.h"

int main() {
    // 1. Initialize the RVSDG graph
    rvsdg_graph* rv_graph = redvisage_graph_create();

    // 2. Build the RVSDG (e.g., a simple computation: x = a * 2)
    // In a real compiler, this is generated from an AST or other IR
    rvsdg_node* node_a = redvisage_make_var(rv_graph, "a");
    rvsdg_node* node_2 = redvisage_make_const_int(rv_graph, 2);
    rvsdg_node* node_mul = redvisage_make_op(rv_graph, RVSDG_OP_MUL, node_a, node_2);
    
    // Set the root of the RVSDG
    redvisage_set_root(rv_graph, node_mul);

    // 3. Initialize Equinox E-Graph
    egraph* eg = equinox_egraph_create();

    // 4. Lower RVSDG into the E-Graph
    // This populates 'eg' and returns the eclass_id of the root node
    eclass_id root_eclass = redvisage_lower_to_egraph(rv_graph, eg);

    // 5. Apply Rewrites (Equality Saturation)
    // E.g., a rule that rewrites (mul ?a 2) -> (shl ?a 1)
    equinox_add_ruleset(eg, redvisage_default_rules());
    equinox_run(eg); 

    // 6. Extract the optimized RVSDG
    // Extracts the cheapest path from the saturated E-Graph
    rvsdg_graph* optimized_graph = redvisage_extract_graph(eg, root_eclass);

    // 7. Use the optimized graph (e.g., code generation or printing)
    printf("Optimization complete. New root node type: %d\n", optimized_graph->root->type);

    // Cleanup
    redvisage_graph_destroy(rv_graph);
    redvisage_graph_destroy(optimized_graph);
    equinox_egraph_destroy(eg);

    return 0;
}
```

# The `src/common` Directory

## Updating Equinox

Inside `src/common` directory, we offer several facilities. All these facilities are handled by `cmake/HandleCommons.cmake`.

## Klib Memory Wrapper (`src/common/memory`, `include/common/memory.h`)

We use this directory to wrap Klib's allocator, arena and memory pool. We have to rewrite aspects of Equinox to integrate this memory with it, but for RedVisage, it's all a virgin forest.

## The EQVSG Language (`src/common/eqvsg`, `include/common/eqvsg.h`)

EQVSG is an S-Expression based language that explicitly models the data flow, state thread, and loop hierarchy:

```eqvsg
;; 's0' is the initial state token
(let i0 0)
(let limit 10)

;; theta signature: (theta [initial_args] (loop_body) [return_values])
(let loop_result 
  (theta [s0, i0] 
    ;; --- LOOP BODY REGION ---
    (region [s_in, i_in]
      ;; Compute condition
      (let cond (less_than i_in limit))
      
      ;; Print takes a state token and returns a new state token
      (let s_next (print s_in i_in))
      
      ;; Increment
      (let i_next (add i_in 1))
      
      ;; Yield dictates whether the loop continues, and what values loop back
      (yield cond [s_next, i_next])
    )
  )
)

;; Extract final state from the theta node's outputs
(let s_final (extract loop_result 0))
```

To realize EQVSG, we use the S7 Scheme. EQVSG is just a binding layer on top of S7 Scheme. I have put S7 Scheme in `third_party`. You need to bind EQVSG into S7 Scheme, and produce an executable that does not seem like it's S7 being embedded! Use `cmake/HandleEQVSG.cmake` to handle linking of S7 and installation of the `eqvsg` binary. This binary has a robust CLI:

```
Usage: eqvsg [OPTIONS] <INPUT_FILE>

Arguments:
  <INPUT_FILE>             Path to the input IL file (.vsg or .il) containing S-expressions.

Options:
  -o, --output <FILE>      Write the optimized RVSDG to <FILE>. If not specified, 
                           defaults to stdout.
  -O, --opt-level <LEVEL>  Set equality saturation limits:
                             0: No optimization (parsing and lowering only).
                             1: Basic rules, node limit 10,000 (Default).
                             2: Aggressive rules, loop unrolling, node limit 50,000.
  -r, --rules <FILE>       Path to a custom E-Graph rewrite ruleset (.rules).
                           Overrides the built-in rules for the specified opt-level.

Debug & Dumping:
  --dump-rvsdg             Print the parsed RVSDG to stderr before lowering (level 1).
  --dump-egraph            Print the E-Graph E-classes to stderr after saturation (level 2).
  --emit-dot <DIR>         Generate Graphviz .dot files for the graph stages in <DIR>.

Execution (Optional if eqvsg includes an interpreter):
  --run                    Interpret the extracted (optimized) RVSDG immediately.
  -a, --args <ARGS>...     Arguments to pass to the interpreted program.

General:
  -v, --verbose            Enable verbose logging (shows eq-sat iteration stats).
  -h, --help               Print this help message.
  -V, --version            Print version information.
```

## The Dump (`src/common/dump`, `include/common/dump.h`)

We can dump an EQVSG program using `--dump`. It has two levels, level 1 and level 2.

Level 1 is RedVisage's RDVSG.


```dump
=== REDVISAGE RVSDG DUMP ===
[Graph ID: rv_01]

-- Main Region --
n0: CONST  val=0
n1: CONST  val=10
n2: CONST  val=1
n3: STATE_INIT

n4: THETA  inputs=[n3, n0]  body_region=r_theta_1

-- Region: r_theta_1 (Loop Body) --
  arg0: STATE_IN
  arg1: VAL_IN

  n5: LESS_THAN lhs=arg1 rhs=n1
  n6: PRINT     state=arg0 val=arg1
  n7: ADD       lhs=arg1 rhs=n2

  n8: YIELD     cond=n5 outputs=[n6, n7]
-----------------------------------

n9: EXTRACT node=n4 index=0 (extracts final state)
============================
```

Level 2 is Equinox's E-graph:

```dump
=== EQUINOX E-GRAPH DUMP ===
[E-Graph Size: 10 classes, 11 nodes]

e1  : { (CONST 0) }
e2  : { (CONST 10) }
e3  : { (CONST 1) }
e4  : { (STATE_INIT) }

e5  : { (STATE_IN) }
e6  : { (VAL_IN) }

e7  : { (LESS_THAN e6 e2) }
e8  : { (PRINT e5 e6) }
e9  : { (ADD e6 e3) }

e10 : { (YIELD e7 e8 e9) }

e11 : { 
        (THETA e4 e1 e10) 
        ;; If equality saturation applied loop unrolling rules, 
        ;; the unrolled sequence would appear here as an equivalent node!
      }

e12 : { (EXTRACT e11 0) }
============================
```


## Rules (`src/common/erl`, `include/common/erl.h`)

We can, yet again, use S7 to define rules. These rule files are passed to `eqvsg` via `--rules` flag. Several can be accepted. A rules files looks like this:

```sexp
;; Basic Algebraic Simplifications
(rewrite "add-zero" (ADD ?x 0) ?x)
(rewrite "mul-one"  (MUL ?x 1) ?x)
(rewrite "mul-zero" (MUL ?x 0) 0)

;; Strength Reduction
(rewrite "mul-two"  (MUL ?x 2) (ADD ?x ?x))
(rewrite "div-two"  (DIV ?x 2) (SHR ?x 1))

;; Commutativity and Associativity
(rewrite "add-comm" (ADD ?x ?y) (ADD ?y ?x))
(rewrite "add-assoc" (ADD (ADD ?x ?y) ?z) (ADD ?x (ADD ?y ?z)))

;; Boolean / Bitwise Logic
(rewrite "and-self" (AND ?x ?x) ?x)
(rewrite "xor-self" (XOR ?x ?x) 0)
(rewrite "not-not"  (NOT (NOT ?x)) ?x)

;; RVSDG / Control Flow Specific (Hypothetical)
;; If the condition of a branch is known to be true, simplify to the true path
(rewrite "branch-true" (GAMMA 1 ?true_region ?false_region) ?true_region)
```

This language is called "EQVSG Rule Language" or ERL.

## Compiler to Assembly (`src/common/compiler`, `include/common/compiler.h`)

To compile EQVSG to Assembly, we use S7 again. We define the following libraries for S7 scheme. EQVSG itself is defined in S7, so we define a function for EQVSG in S7 called "(dump-bitcode "foo.bin")". This bitcide is then used to assemble the code:

```
$ eqvsg assemble foo.evg --profile=foo_machine.eqp --out foo.s
```

***EQP: Assembly Profiles***

- `define-isa`: with this procedure, we define the target architecture's ISA. We could also load it from an S-Expression ISA file: `(define-isa (load-json "wasm.sxi"))`. We offer tools to convert JSON and YAML into compliant SXI files. It usually looks like this:

```sxi
(define-isa
  (instructions
    (instr "add"
      (pattern (ADD ?x ?y))
      (inputs (reg reg))
      (outputs (reg))
      (cost 1))

    (instr "addi"
      (pattern (ADD ?x imm12))
      (inputs (reg imm))
      (outputs (reg))
      (cost 1))

    (instr "shl1"
      (pattern (MUL ?x 2))
      (inputs (reg))
      (outputs (reg))
      (cost 1))))
```

The extra information is for instruction selection.

- `select-instructions`: before matching, we canonicalize the EQVSG into a form that is easy to lower:
  - explicit constants
  - explicit state token dependencies
  - explicit region arguments
  - explicit extracts from multi-result nodes
  - lowered `gammar`/`thetha` region structure;
We then cover the EQVSG using a greedy/subgraph matching algorithm:
   - For each node:
      - try the most specific pattern first
      - if one matches, emit the instruction
      - otherwise, fall back to more general pattern
As for Gamma nd Theta, they will look something like this at the end:

```asm
;; gamma
cmp cond, 0
je false_label
true_label:
  ...
  jmp join_label
false_label:
  ...
join_label:

;; theta
mov s, s0
mov i, i0
loop_header:
  ...
  ; body
  ...
  ; cond in rcond
  test rcond, rcond
  jnz loop_header
loop_exit:
  ; final s, i available
  ...


- Register Allocation: We use a simple linear scan algorithm for register allocation. After register allocation, we emit the Assembly .s file.

The program is shipped with `evprofiles/x86-64.evp` and `evprofiles/x86-64.sxa`. It's your job to generate both.
