cmake_minimum_required(VERSION 3.30.0..)

project(JustReflectMe VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_policy(SET CMP0141 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
)

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    if (NOT CMAKE_CXX_COMPILER_LAUNCHER)
        find_program(CCACHE_PROGRAM ccache)
        if (CCACHE_PROGRAM)
            set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
            set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
            message(STATUS "ccache: ${CCACHE_PROGRAM}")
        endif ()
    endif ()
endif ()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Features.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Coverage.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ========= Base setup & Predefinitions =========
option(JRM_ENABLE_CODE_COVERAGE "Instrument project targets for code coverage collection." OFF)
option(JRM_GENERATE_CODE_COVERAGE_HTML
       "Add the JRMCodeCoverageHtml target that runs tests and writes an HTML report."
       OFF)
set(JRM_CODE_COVERAGE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/coverage" CACHE PATH
    "Directory for the JRM code coverage HTML report.")
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

JRMConfigureCodeCoverage()
