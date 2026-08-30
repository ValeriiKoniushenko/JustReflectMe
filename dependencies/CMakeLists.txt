
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

add_subdirectory(MiniYaml)

if (JRM_ENABLE_BENCHMARKS)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    add_subdirectory(benchmark)
endif ()

if (NOT JRM_DISABLE_TESTS)
    add_subdirectory(googletest)
endif ()

add_subdirectory(NlohmannJson)

set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

