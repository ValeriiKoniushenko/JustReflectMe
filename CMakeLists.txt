cmake_minimum_required(VERSION 3.30.0..)

project(JustReflectMe VERSION 1.2.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
        message(STATUS "ccache: ${CCACHE_PROGRAM}")
    endif ()

    if (MSVC AND CCACHE_PROGRAM)
        foreach (flag_var
            CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELWITHDEBINFO)
            string(REPLACE "/Zi" "/Z7" ${flag_var} "${${flag_var}}")
        endforeach ()
    endif ()
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Features.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flags.cmake)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
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
add_subdirectory(dependencies)

if (NOT JRM_DISABLE_TESTS)
    add_subdirectory(tests)
endif ()

add_subdirectory(sources)
