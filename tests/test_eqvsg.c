#include <catch2/catch_test_macros.hpp>

#include "common/eqvsg.h"

#include <cstdio>

TEST_CASE("eqvsg parses an existing file", "[eqvsg]") {
    const char* path = "/tmp/eqvsg_test.il";
    FILE* f = std::fopen(path, "w");
    REQUIRE(f != nullptr);
    std::fputs("(let x 0)\n", f);
    std::fclose(f);

    rvsdg_graph_t* g = nullptr;
    REQUIRE(redvisage_eqvsg_parse_file(path, &g) == 0);
    REQUIRE(g != nullptr);
    REQUIRE(g->root != nullptr);
    redvisage_graph_destroy(g);
}
