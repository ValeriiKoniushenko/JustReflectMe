
if (NOT JRM_DISABLE_TESTS)

    add_subdirectory(uat)

    add_executable(JRMTests BaseFixture.cpp FileProcessorTests.cpp main.cpp)
    target_link_libraries(JRMTests PUBLIC gtest gmock JustReflectMe::Core)
    target_compile_features(JRMTests PUBLIC ${CXX_STANDARD})
    target_compile_definitions(JRMTests PRIVATE JRM_TEST_RESOURCES=${CMAKE_CURRENT_SOURCE_DIR}/test_resources)
    JRFCoreAddCompileOptionsTo(JRMTests)

endif ()