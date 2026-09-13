include_guard(GLOBAL)

function(JRMApplyTargetSettings target)
    target_compile_features(${target} PUBLIC cxx_std_26)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        COMPILE_WARNING_AS_ERROR "${JRM_WARNINGS_AS_ERRORS}")
    if(WIN32)
        target_compile_definitions(${target} PRIVATE NOMINMAX=1)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /we4715)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror=return-type)
    endif()
endfunction()
