include_guard()

function(JRMConfigureCodeCoverage)
    if (JRM_GENERATE_CODE_COVERAGE_HTML AND NOT JRM_ENABLE_CODE_COVERAGE)
        message(FATAL_ERROR
            "JRM_GENERATE_CODE_COVERAGE_HTML requires JRM_ENABLE_CODE_COVERAGE=ON.")
    endif ()

    if (NOT JRM_ENABLE_CODE_COVERAGE)
        return()
    endif ()

    if (JRM_DISABLE_TESTS)
        message(FATAL_ERROR
            "JRM_ENABLE_CODE_COVERAGE requires tests. Configure with JRM_DISABLE_TESTS=OFF.")
    endif ()

    foreach (target IN ITEMS _JustReflectMe_Adapter _JustReflectMe_Core jrm JRMTests)
        if (NOT TARGET ${target})
            message(FATAL_ERROR
                "JRM_ENABLE_CODE_COVERAGE expected target '${target}', but it was not created.")
        endif ()
    endforeach ()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(JRM_CODE_COVERAGE_COMPILE_OPTIONS --coverage)
        set(JRM_CODE_COVERAGE_LINK_OPTIONS --coverage)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(JRM_CODE_COVERAGE_COMPILE_OPTIONS
            -fprofile-instr-generate
            -fcoverage-mapping
        )
        set(JRM_CODE_COVERAGE_LINK_OPTIONS -fprofile-instr-generate)
    else ()
        message(FATAL_ERROR
            "JRM_ENABLE_CODE_COVERAGE supports GCC and Clang only. Current compiler: "
            "${CMAKE_CXX_COMPILER_ID}.")
    endif ()

    foreach (target IN ITEMS _JustReflectMe_Adapter _JustReflectMe_Core JRMTests)
        target_compile_options(${target} PRIVATE ${JRM_CODE_COVERAGE_COMPILE_OPTIONS})
    endforeach ()

    foreach (target IN ITEMS jrm JRMTests)
        target_link_options(${target} PRIVATE ${JRM_CODE_COVERAGE_LINK_OPTIONS})
    endforeach ()

    if (NOT JRM_GENERATE_CODE_COVERAGE_HTML)
        return()
    endif ()

    cmake_path(ABSOLUTE_PATH JRM_CODE_COVERAGE_OUTPUT_DIRECTORY NORMALIZE
        OUTPUT_VARIABLE JRM_CODE_COVERAGE_OUTPUT_DIRECTORY_ABSOLUTE)
    cmake_path(IS_PREFIX CMAKE_BINARY_DIR "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY_ABSOLUTE}" NORMALIZE
        JRM_CODE_COVERAGE_OUTPUT_IS_IN_BUILD_DIRECTORY)
    if (NOT JRM_CODE_COVERAGE_OUTPUT_IS_IN_BUILD_DIRECTORY
        OR "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY_ABSOLUTE}" STREQUAL "${CMAKE_BINARY_DIR}")
        message(FATAL_ERROR
            "JRM_CODE_COVERAGE_OUTPUT_DIRECTORY must be a child of CMAKE_BINARY_DIR. "
            "Current value: ${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}")
    endif ()

    set(JRM_CODE_COVERAGE_OUTPUT_DIRECTORY
        "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY_ABSOLUTE}")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        find_program(JRM_GCOVR_EXECUTABLE NAMES gcovr)
        if (NOT JRM_GCOVR_EXECUTABLE)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires gcovr when using GCC.")
        endif ()

        execute_process(
            COMMAND "${JRM_GCOVR_EXECUTABLE}" --help
            RESULT_VARIABLE JRM_GCOVR_HELP_RESULT
            OUTPUT_VARIABLE JRM_GCOVR_HELP_OUTPUT
            ERROR_VARIABLE JRM_GCOVR_HELP_ERROR
        )
        if (NOT JRM_GCOVR_HELP_RESULT EQUAL 0)
            message(FATAL_ERROR
                "Failed to inspect gcovr capabilities. Details: ${JRM_GCOVR_HELP_ERROR}")
        endif ()

        string(FIND "${JRM_GCOVR_HELP_OUTPUT}" "--exclude-throw-branches"
               JRM_GCOVR_EXCLUDE_THROW_BRANCHES_POSITION)
        if (JRM_GCOVR_EXCLUDE_THROW_BRANCHES_POSITION EQUAL -1)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires a gcovr version that supports "
                "--exclude-throw-branches.")
        endif ()

        string(FIND "${JRM_GCOVR_HELP_OUTPUT}" "--exclude-unreachable-branches"
               JRM_GCOVR_EXCLUDE_UNREACHABLE_BRANCHES_POSITION)
        if (JRM_GCOVR_EXCLUDE_UNREACHABLE_BRANCHES_POSITION EQUAL -1)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires a gcovr version that supports "
                "--exclude-unreachable-branches.")
        endif ()

        find_program(JRM_GCOV_EXECUTABLE NAMES gcov)
        if (NOT JRM_GCOV_EXECUTABLE)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires gcov when using GCC.")
        endif ()

        add_custom_target(JRMCodeCoverageHtml
            COMMAND "${CMAKE_COMMAND}" -E rm -rf "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}"
            COMMAND "$<TARGET_FILE:JRMTests>"
            COMMAND "${JRM_GCOVR_EXECUTABLE}"
            --root "${PROJECT_SOURCE_DIR}"
            --filter "${PROJECT_SOURCE_DIR}/sources"
            --exclude "${CMAKE_BINARY_DIR}"
            --gcov-executable "${JRM_GCOV_EXECUTABLE}"
            --exclude-throw-branches
            --exclude-unreachable-branches
            --html-details "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}/index.html"
            --print-summary
            DEPENDS JRMTests
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            BYPRODUCTS "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}/index.html"
            USES_TERMINAL
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        find_program(JRM_LLVM_PROFDATA_EXECUTABLE NAMES llvm-profdata)
        if (NOT JRM_LLVM_PROFDATA_EXECUTABLE)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires llvm-profdata when using Clang.")
        endif ()

        find_program(JRM_LLVM_COV_EXECUTABLE NAMES llvm-cov)
        if (NOT JRM_LLVM_COV_EXECUTABLE)
            message(FATAL_ERROR
                "JRM_GENERATE_CODE_COVERAGE_HTML requires llvm-cov when using Clang.")
        endif ()

        set(JRM_CODE_COVERAGE_RAW_PROFILE
            "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}/jrmtests.profraw")
        set(JRM_CODE_COVERAGE_INDEXED_PROFILE
            "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}/jrmtests.profdata")

        add_custom_target(JRMCodeCoverageHtml
            COMMAND "${CMAKE_COMMAND}" -E rm -rf "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}"
            COMMAND "${CMAKE_COMMAND}" -E env
            "LLVM_PROFILE_FILE=${JRM_CODE_COVERAGE_RAW_PROFILE}"
            "$<TARGET_FILE:JRMTests>"
            COMMAND "${JRM_LLVM_PROFDATA_EXECUTABLE}" merge -sparse
            "${JRM_CODE_COVERAGE_RAW_PROFILE}"
            -o "${JRM_CODE_COVERAGE_INDEXED_PROFILE}"
            COMMAND "${JRM_LLVM_COV_EXECUTABLE}" show "$<TARGET_FILE:JRMTests>"
            "-instr-profile=${JRM_CODE_COVERAGE_INDEXED_PROFILE}"
            -format=html
            "-output-dir=${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}"
            "-ignore-filename-regex=.*/dependencies/.*"
            DEPENDS JRMTests
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            BYPRODUCTS "${JRM_CODE_COVERAGE_OUTPUT_DIRECTORY}/index.html"
            USES_TERMINAL
        )
    endif ()
endfunction()
