include(FetchContent)

function(JRF_SuppressAllSubmoduleWarnings Target)
    target_compile_options(${Target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/w>
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-w>
    )
endfunction()

if (NOT JRM_DISABLE_TESTS)

    FetchContent_Declare(GoogleTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(GoogleTest)
    JRF_SuppressAllSubmoduleWarnings(gtest)

endif ()