
if (NOT JRM_DISABLE_TESTS)

    file(
        GLOB Sources
        "*.cpp"
    )

    add_executable(JRMTests ${Sources})
    target_link_libraries(JRMTests PUBLIC gtest JustReflectMe::Core)
    target_compile_features(JRMTests PUBLIC ${CXX_STANDARD})

    JRFCoreAddCompileOptionsTo(JRMTests)

endif ()