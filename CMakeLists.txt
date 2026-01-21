cmake_minimum_required(VERSION 3.30.0..)

project(JustReflectMe VERSION 0.9.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Features.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flags.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ========= Base setup & Predefinitions =========
set(BUILD_UTILS_WITH_TEST_COVERAGE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(REQUIRED_CXX_STANDARD cxx_std_23) # read-only
set(CXX_STANDARD)

JRF_InstallCXXStandardVersion()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/JustReflectMe/version.h.template
    ${CMAKE_CURRENT_SOURCE_DIR}/sources/JustReflectMe/version.h
    @ONLY
)

# ========= Adding subdirectories =========
# Adding dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchDependencies.cmake)

add_subdirectory(dependencies/MiniYaml)

# Adding core
if (NOT JRM_DISABLE_TESTS)
    add_subdirectory(tests)
endif ()

add_subdirectory(sources)
