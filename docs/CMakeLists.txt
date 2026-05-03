# Find Doxygen
find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(WARNING "Doxygen not found. Documentation will not be built.")
    return()
endif()

# Configure Doxyfile
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

# Add custom target for documentation
add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

# Install documentation
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    OPTIONAL
)
