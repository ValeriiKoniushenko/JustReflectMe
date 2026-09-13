add_executable(JRMTests
    main.cpp
    AdapterTests.cpp
    BaseReflectorTests.cpp
    ConfigAndCacheTests.cpp
    EnumsAndFileDataTests.cpp
    FileProcessorTests.cpp
    GenerationPipelineTests.cpp
    ScopesTests.cpp
    StringAndNavigationTests.cpp
    uat/Classes.cpp
    uat/Enums.cpp
    uat/MltClasses.cpp
    TestSupport.h
)
target_link_libraries(JRMTests PRIVATE GTest::gtest GTest::gmock JustReflectMe::Core JustReflectMe::Adapter)
JRMApplyTargetSettings(JRMTests)
jrm_target_reflection(JRMTests
    ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/.jrm/config.yaml"
    HEADERS uat/Classes.h uat/Enums.h uat/MltClasses.h)

# Ensure the directory exists before CTest runs
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/run/$<CONFIG>/.keep" CONTENT "")
add_test(NAME JRMTests COMMAND JRMTests)
set_tests_properties(JRMTests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/run/$<CONFIG>" TIMEOUT 60)
