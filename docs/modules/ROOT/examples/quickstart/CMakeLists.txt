cmake_minimum_required(VERSION 3.30)
project(jrm_quickstart LANGUAGES CXX)

# This path makes the checked-in documentation example build from this repository.
set(JRM_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../" CACHE PATH
    "Path to a JustReflectMe checkout")

# tag::dependency[]
set(JRM_DISABLE_TESTS ON)
add_subdirectory("${JRM_SOURCE_DIR}" jrm-build)

add_executable(jrm_quickstart main.cpp)
target_link_libraries(jrm_quickstart PRIVATE JustReflectMe::Adapter)
# end::dependency[]

# tag::generator[]
add_custom_target(jrm_quickstart_codegen ALL
    COMMAND $<TARGET_FILE:jrm> "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS jrm
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)
add_dependencies(jrm_quickstart jrm_quickstart_codegen)
# end::generator[]
