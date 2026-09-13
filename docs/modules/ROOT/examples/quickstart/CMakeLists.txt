cmake_minimum_required(VERSION 3.30)
project(jrm_quickstart LANGUAGES CXX)

set(JRM_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../" CACHE PATH
    "Path to a JustReflectMe checkout")

# tag::dependency[]
set(JRM_DISABLE_TESTS ON)
add_subdirectory("${JRM_SOURCE_DIR}" jrm-build)

add_executable(jrm_quickstart)
target_link_libraries(jrm_quickstart PRIVATE JustReflectMe::Adapter)
set_target_properties(jrm_quickstart PROPERTIES CXX_EXTENSIONS OFF)
# end::dependency[]

# tag::generator[]
jrm_target_reflection(jrm_quickstart
    ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCES main.cpp
    HEADERS model.h)
add_custom_target(jrm_quickstart_codegen DEPENDS jrm_quickstart_CodeReflector)
# end::generator[]
