cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(LittleTypeLibrary
        VERSION 0.0.1
        LANGUAGES CXX)

# ---- Includes ----

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# ---- Install directories ----

set(littletypelibrary_directory "littletypelibrary-${PROJECT_VERSION}")
set(littletypelibrary_include_directory "${CMAKE_INSTALL_INCLUDEDIR}/${littletypelibrary_directory}")

# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(littletypelibrary_warning_guard "SYSTEM")
if(LittleTypeLibrary_INCLUDE_WITHOUT_SYSTEM)
  set(littletypelibrary_warning_guard "")
endif()

# ---- Declare library ----

add_library(LTL INTERFACE)
add_library(LittleTypeLibrary::LTL ALIAS LTL)

target_include_directories(LTL
        ${littletypelibrary_warning_guard}
        INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_compile_features(LTL INTERFACE cxx_std_17)
target_compile_options(LTL INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)


# ---- Install ----

install(TARGETS LTL
        EXPORT LittleTypeLibraryTargets
        INCLUDES DESTINATION "${littletypelibrary_include_directory}")

set(littletypelibrary_install_cmakedir
        "${CMAKE_INSTALL_LIBDIR}/cmake/${littletypelibrary_directory}")

write_basic_package_version_file(
        LittleTypeLibraryConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT)

install(EXPORT LittleTypeLibraryTargets
        NAMESPACE LittleTypeLibrary::
        DESTINATION "${littletypelibrary_install_cmakedir}")

install(FILES
        "${PROJECT_SOURCE_DIR}/cmake/LittleTypeLibraryConfig.cmake"
        "${PROJECT_BINARY_DIR}/LittleTypeLibraryConfigVersion.cmake"
        DESTINATION "${littletypelibrary_install_cmakedir}")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "${littletypelibrary_include_directory}")
