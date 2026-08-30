add_executable(JRMStringAndNavigationBenchmarks StringAndNavigationBenchmarks.cpp)
target_link_libraries(JRMStringAndNavigationBenchmarks
    PRIVATE
    JustReflectMe::Core
    benchmark::benchmark_main
)
target_compile_features(JRMStringAndNavigationBenchmarks PRIVATE ${CXX_STANDARD})
JRFCoreAddCompileOptionsTo(JRMStringAndNavigationBenchmarks)
