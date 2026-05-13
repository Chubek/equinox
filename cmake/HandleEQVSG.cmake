add_executable(eqvsg src/common/eqvsg/main.c)
if(TARGET equinox_shared)
    target_link_libraries(eqvsg PRIVATE equinox_shared)
elseif(TARGET equinox_static)
    target_link_libraries(eqvsg PRIVATE equinox_static)
endif()

target_include_directories(eqvsg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

install(TARGETS eqvsg RUNTIME DESTINATION bin)
