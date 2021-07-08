find_package(Doxygen)

if(DOXYGEN_FOUND)
    # set input and output files
    set(DOCUMENTATION_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../src/ ${CMAKE_CURRENT_SOURCE_DIR}/../documentation/")
    set(DOCUMENTATION_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../docs/)
    set(DOCUMENTATION_MAINPAGE_MD ${CMAKE_CURRENT_SOURCE_DIR}/../README.md)
    set(DOCUMENTATION_IMAGE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../asset)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(Doxyfile.in Doxyfile @ONLY)
    message("Doxygen build started")

    add_custom_target(Documentation ALL
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
endif()
