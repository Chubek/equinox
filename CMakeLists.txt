cmake_minimum_required(VERSION 3.15)
project(equinox VERSION 1.0.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(EQX_BUILD_TESTS "Build tests" ON)
option(EQX_BUILD_DOCS "Build documentation" ON)
option(EQX_BUILD_SHARED "Build shared library" ON)
option(EQX_BUILD_STATIC "Build static library" ON)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Third-party libraries
add_subdirectory(third_party/uthash EXCLUDE_FROM_ALL)
add_subdirectory(third_party/tlsf EXCLUDE_FROM_ALL)
add_subdirectory(third_party/libdag-ethash EXCLUDE_FROM_ALL)

# Source files
set(EQUINOX_SOURCES
    src/enode.c
    src/eclass.c
    src/egraph.c
    src/unionfind.c
    src/hashcons.c
    src/rewrite.c
)

# Header files
set(EQUINOX_HEADERS
    include/equinox/equinox.h
    include/equinox/enode.h
    include/equinox/eclass.h
    include/equinox/egraph.h
    include/equinox/unionfind.h
    include/equinox/hashcons.h
    include/equinox/rewrite.h
    include/equinox/libutils.h
)

# Shared library
if(EQX_BUILD_SHARED)
    add_library(equinox SHARED ${EQUINOX_SOURCES})
    target_include_directories(equinox PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(equinox PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        PUBLIC_HEADER "${EQUINOX_HEADERS}"
    )
endif()

# Static library
if(EQX_BUILD_STATIC)
    add_library(equinox_static STATIC ${EQUINOX_SOURCES})
    target_include_directories(equinox_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(equinox_static PROPERTIES
        OUTPUT_NAME equinox
        PUBLIC_HEADER "${EQUINOX_HEADERS}"
    )
endif()

# Tests
if(EQX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation
if(EQX_BUILD_DOCS)
    add_subdirectory(docs)
endif()

# Install rules
include(GNUInstallDirs)

if(EQX_BUILD_SHARED)
    install(TARGETS equinox
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/equinox
    )
endif()

if(EQX_BUILD_STATIC)
    install(TARGETS equinox_static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/equinox
    )
endif()

# pkg-config
configure_file(equinox.pc.in equinox.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/equinox.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
