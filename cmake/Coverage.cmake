include_guard(GLOBAL)

function(JRMConfigureCodeCoverage)
    if(NOT JRM_ENABLE_CODE_COVERAGE)
        message(STATUS "Code coverage - disabled")
        return()
    endif()
    message(STATUS "Code coverage - activated")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(compile_options --coverage)
        set(link_options --coverage)
    else()
        set(compile_options -fprofile-instr-generate -fcoverage-mapping)
        set(link_options -fprofile-instr-generate)
    endif()
    foreach(target IN ITEMS _JustReflectMe_Adapter _JustReflectMe_Core)
        target_compile_options(${target} PRIVATE ${compile_options})
        target_link_options(${target} INTERFACE ${link_options})
    endforeach()
    target_compile_options(JRMTests PRIVATE ${compile_options})
    target_link_options(JRMTests PRIVATE ${link_options})
    if(NOT JRM_GENERATE_CODE_COVERAGE_HTML)
        message(STATUS "Code coverage HTML generation - disabled")
        return()
    endif()
    message(STATUS "Code coverage HTML generation - activated")

    cmake_path(ABSOLUTE_PATH JRM_CODE_COVERAGE_OUTPUT_DIRECTORY
        BASE_DIRECTORY "${PROJECT_BINARY_DIR}" NORMALIZE OUTPUT_VARIABLE report)
    cmake_path(IS_PREFIX PROJECT_BINARY_DIR "${report}" NORMALIZE contained)
    if(NOT contained OR report STREQUAL PROJECT_BINARY_DIR)
        message(FATAL_ERROR "JRM_CODE_COVERAGE_OUTPUT_DIRECTORY must be a proper child of PROJECT_BINARY_DIR.")
    endif()
    set(protected "${PROJECT_BINARY_DIR}/CMakeFiles" "${PROJECT_BINARY_DIR}/sources"
        "${PROJECT_BINARY_DIR}/tests" "${PROJECT_BINARY_DIR}/dependencies"
        "${PROJECT_BINARY_DIR}/include" "${PROJECT_BINARY_DIR}/benchmarks"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    foreach(path IN LISTS protected)
        cmake_path(IS_PREFIX path "${report}" NORMALIZE inside)
        cmake_path(IS_PREFIX report "${path}" NORMALIZE ancestor)
        if(inside OR ancestor)
            message(FATAL_ERROR "Coverage report directory overlaps build artifacts: ${path}")
        endif()
    endforeach()
    if(CMAKE_CONFIGURATION_TYPES)
        string(APPEND report "/$<CONFIG>")
    endif()
    set(multi_config OFF)
    if(CMAKE_CONFIGURATION_TYPES)
        set(multi_config ON)
    endif()
    set(prepare COMMAND "${CMAKE_COMMAND}" "-DJRM_REPORT=${report}"
        "-DJRM_BINARY_DIR=${PROJECT_BINARY_DIR}"
        -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/PrepareCoverage.cmake")
    set(report_files "${report}/index.html")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        find_program(JRM_GCOVR_EXECUTABLE NAMES gcovr REQUIRED)
        string(REGEX MATCH "^[0-9]+" compiler_major "${CMAKE_CXX_COMPILER_VERSION}")
        get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        find_program(JRM_GCOV_EXECUTABLE NAMES "gcov-${compiler_major}" gcov HINTS "${compiler_dir}" REQUIRED)
        execute_process(COMMAND "${JRM_GCOV_EXECUTABLE}" --version
            OUTPUT_VARIABLE tool_version COMMAND_ERROR_IS_FATAL ANY)
        if(NOT tool_version MATCHES " ${compiler_major}\\.[0-9]+")
            message(FATAL_ERROR "gcov must match GCC ${compiler_major}: ${JRM_GCOV_EXECUTABLE}")
        endif()
        execute_process(COMMAND "${JRM_GCOVR_EXECUTABLE}" --help
            OUTPUT_VARIABLE help COMMAND_ERROR_IS_FATAL ANY)
        foreach(flag IN ITEMS --exclude-throw-branches --exclude-unreachable-branches
                --html-single-page --html-self-contained --json-summary)
            if(NOT help MATCHES "${flag}")
                message(FATAL_ERROR "gcovr does not support required option ${flag}")
            endif()
        endforeach()
        list(APPEND report_files "${report}/summary.json")
        # Reset only this configuration's instrumented objects before collecting counters.
        set(reset COMMAND "${CMAKE_COMMAND}" "-DJRM_BINARY_DIR=${PROJECT_BINARY_DIR}"
            "-DJRM_CONFIG=$<CONFIG>" "-DJRM_MULTI_CONFIG=${multi_config}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ResetGcov.cmake")
        set(object_directories)
        foreach(directory IN ITEMS sources/CMakeFiles/_JustReflectMe_Adapter.dir
                sources/CMakeFiles/_JustReflectMe_Core.dir tests/CMakeFiles/JRMTests.dir)
            if(CMAKE_CONFIGURATION_TYPES)
                string(APPEND directory "/$<CONFIG>")
            endif()
            list(APPEND object_directories "${PROJECT_BINARY_DIR}/${directory}")
        endforeach()
        set(collect COMMAND "${JRM_GCOVR_EXECUTABLE}" ${object_directories}
            --root "${PROJECT_SOURCE_DIR}" --filter "${PROJECT_SOURCE_DIR}/sources/"
            --gcov-executable "${JRM_GCOV_EXECUTABLE}"
            --exclude-throw-branches --exclude-unreachable-branches
            --json-summary "${report}/summary.json" --html-details "${report}/index.html"
            --html-single-page --html-self-contained --print-summary)
        set(run COMMAND "$<TARGET_FILE:JRMTests>")
    else()
        string(REGEX MATCH "^[0-9]+" compiler_major "${CMAKE_CXX_COMPILER_VERSION}")
        get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        find_program(JRM_LLVM_PROFDATA_EXECUTABLE NAMES "llvm-profdata-${compiler_major}" llvm-profdata
            HINTS "${compiler_dir}" REQUIRED)
        find_program(JRM_LLVM_COV_EXECUTABLE NAMES "llvm-cov-${compiler_major}" llvm-cov
            HINTS "${compiler_dir}" REQUIRED)
        foreach(tool IN ITEMS JRM_LLVM_PROFDATA_EXECUTABLE JRM_LLVM_COV_EXECUTABLE)
            execute_process(COMMAND "${${tool}}" --version
                OUTPUT_VARIABLE tool_version COMMAND_ERROR_IS_FATAL ANY)
            if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
               AND NOT tool_version MATCHES "version ${compiler_major}\\.")
                message(FATAL_ERROR "${tool} must match Clang ${compiler_major}.")
            endif()
        endforeach()
        set(run COMMAND "${CMAKE_COMMAND}" -E env "LLVM_PROFILE_FILE=${report}/jrmtests.profraw"
            "$<TARGET_FILE:JRMTests>")
        set(collect
            COMMAND "${JRM_LLVM_PROFDATA_EXECUTABLE}" merge -sparse "${report}/jrmtests.profraw"
                -o "${report}/jrmtests.profdata"
            COMMAND "${JRM_LLVM_COV_EXECUTABLE}" show "$<TARGET_FILE:JRMTests>"
                "-instr-profile=${report}/jrmtests.profdata" -format=html
                "-output-dir=${report}" "-ignore-filename-regex=.*/(dependencies|tests)/.*")
        list(APPEND report_files "${report}/jrmtests.profraw" "${report}/jrmtests.profdata")
    endif()
    add_custom_target(JRMCodeCoverageHtml
        ${prepare} ${reset} ${run} ${collect}
        DEPENDS JRMTests
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/tests/run/$<CONFIG>"
        BYPRODUCTS ${report_files}
        COMMAND_EXPAND_LISTS VERBATIM USES_TERMINAL)
endfunction()
