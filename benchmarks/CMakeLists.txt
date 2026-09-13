add_executable(JRMStringAndNavigationBenchmarks StringAndNavigationBenchmarks.cpp)
target_link_libraries(JRMStringAndNavigationBenchmarks
    PRIVATE
    JustReflectMe::Core
    benchmark::benchmark_main
)
JRMApplyTargetSettings(JRMStringAndNavigationBenchmarks)
