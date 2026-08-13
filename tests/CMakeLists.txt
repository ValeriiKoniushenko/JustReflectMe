
if (NOT JRM_DISABLE_TESTS)

    FILE(GLOB_RECURSE SOURCES "*.cpp")
    FILE(GLOB_RECURSE HEADERS "*.h")

    add_executable(JRMTests ${SOURCES} ${HEADERS})
    target_link_libraries(JRMTests PUBLIC gtest gmock JustReflectMe::Core)
    target_compile_features(JRMTests PUBLIC ${CXX_STANDARD})
    add_custom_target(JRMTests_CodeReflector
        COMMAND jrm ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies(JRMTests JRMTests_CodeReflector)

    JRFCoreAddCompileOptionsTo(JRMTests)

endif ()