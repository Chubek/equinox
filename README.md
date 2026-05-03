# Equinox: E-Graph Library for C

Equinox is an e-graph library, written in C, for finding the equality saturation of terms and expressions, with bindings provided via SWIG and several wrappers presented based on the bindings. An e-graph of an "Equivalence Graph" is a graph that maintains the equivalence classes of expressions. It is useful specifically in term-rewriting systems.

In an e-graph, nodes are expressions (ENodes) and edges are "child" references between them. Edges point to the grouping of expressions that a node representing that expression (EClass) belongs to. Together, they create an Equivalence graph (EGraph). Each ENode belongs to exactly one EClass.

The main components of an e-graph are a union-find data structure, a term DAG, and a rewrite engine. In a proper e-graph implementation, hashconsing is used for structural sharing. Congruence closure is the key mechanism underlying an e-graph. It is the process of computing all equalities that must hold between expressions based on reflexivity, symmetry, transitivity, and congruence. By grouping equivalent terms into sets called e-classes, the e-graph can represent an exponential number of equivalent expressions in a compact, shared structure. When two terms are merged, the congruence closure algorithm automatically propagates this information upward to ensure that all parent expressions are updated accordingly.

The Equinox API is a bit clumsy in C, that is why we wish to provide as many bindings and wrappers for Equinox. We use primarily SWIG to generate bindings, and use those bindings to create wrappers.

The C++ wrapper is built directlyo on top of the Equinox library, and it offera metaprogramming through DSL. Specifically, we use expression templates to offer an S-Expression-based native DSL.

