# Find or fetch Catch2
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    message(STATUS "Catch2 not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# Test sources
set(TEST_SOURCES
    test_enode.c
    test_eclass.c
    test_egraph.c
    test_unionfind.c
    test_hashcons.c
    test_rewrite.c
)

# Create test executable for each test file
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    
    # Link against equinox library
    if(TARGET equinox)
        target_link_libraries(${test_name} PRIVATE equinox Catch2::Catch2WithMain)
    else()
        target_link_libraries(${test_name} PRIVATE equinox_static Catch2::Catch2WithMain)
    endif()
    
    # Add to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Optional: Create single test runner
add_executable(run_all_tests
    test_enode.c
    test_eclass.c
    test_egraph.c
    test_unionfind.c
    test_hashcons.c
    test_rewrite.c
)

if(TARGET equinox)
    target_link_libraries(run_all_tests PRIVATE equinox Catch2::Catch2WithMain)
else()
    target_link_libraries(run_all_tests PRIVATE equinox_static Catch2::Catch2WithMain)
endif()

add_test(NAME all_tests COMMAND run_all_tests)
