include(FetchContent)

function(JRF_SuppressAllSubmoduleWarnings Target)
    target_compile_options(${Target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/w>
        $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-w>
    )
endfunction()


FetchContent_Declare(PCRE2
        GIT_REPOSITORY https://github.com/PCRE2Project/pcre2.git
        GIT_TAG pcre2-10.45
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(PCRE2_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(PCRE2)
JRF_SuppressAllSubmoduleWarnings(pcre2-8-static)
