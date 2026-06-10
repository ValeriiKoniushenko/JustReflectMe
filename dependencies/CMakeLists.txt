
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

add_subdirectory(MiniYaml)

# ── GoogleTest ───────────────────────────────────────────────────────────────
if(NOT JRM_DISABLE_TESTS)
    add_subdirectory(googletest)
endif()

set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
