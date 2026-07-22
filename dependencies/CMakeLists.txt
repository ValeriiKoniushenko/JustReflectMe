
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

add_subdirectory(MiniYaml)

if (NOT JRM_DISABLE_TESTS)
    add_subdirectory(googletest)
endif ()

add_subdirectory(NlohmannJson)

set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

