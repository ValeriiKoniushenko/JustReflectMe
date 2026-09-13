include_guard(GLOBAL)

# Stage both translation units and headers so quoted includes resolve in the build tree.
function(jrm_target_reflection target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ROOT;CONFIG" "SOURCES;HEADERS")
    if(arg_UNPARSED_ARGUMENTS OR arg_KEYWORDS_MISSING_VALUES OR NOT arg_ROOT OR NOT arg_HEADERS)
        message(FATAL_ERROR "jrm_target_reflection requires ROOT, HEADERS and optional SOURCES/CONFIG.")
    endif()
    cmake_path(ABSOLUTE_PATH arg_ROOT BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE)
    if(arg_CONFIG)
        cmake_path(ABSOLUTE_PATH arg_CONFIG BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE)
    endif()
    set(stage "${CMAKE_CURRENT_BINARY_DIR}/${target}-reflection/$<CONFIG>")
    set(outputs "${stage}/reflection.stamp")
    set(inputs)
    set(commands)
    foreach(file IN LISTS arg_SOURCES arg_HEADERS)
        if(IS_ABSOLUTE "${file}" OR file MATCHES "(^|/)\\.\\.(/|$)")
            message(FATAL_ERROR "Reflection input must be relative to ROOT: ${file}")
        endif()
        get_filename_component(directory "${file}" DIRECTORY)
        list(APPEND commands
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${stage}/${directory}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${arg_ROOT}/${file}" "${stage}/${file}")
        list(APPEND inputs "${arg_ROOT}/${file}")
        list(APPEND outputs "${stage}/${file}")
    endforeach()
    set(generated_headers)
    foreach(header IN LISTS arg_HEADERS)
        get_filename_component(directory "${header}" DIRECTORY)
        get_filename_component(stem "${header}" NAME_WLE)
        list(APPEND generated_headers "${stage}/${directory}/${stem}.generated.h")
    endforeach()
    list(APPEND outputs ${generated_headers})
    set(byproducts "${stage}/.jrm/config.yaml")
    if(arg_CONFIG)
        list(APPEND inputs "${arg_CONFIG}")
        list(APPEND commands
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${stage}/.jrm"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${arg_CONFIG}" "${stage}/.jrm/config.yaml")
    endif()
    add_custom_command(OUTPUT ${outputs}
        ${commands}
        COMMAND "${CMAKE_COMMAND}" -E rm -f "${stage}/.jrm/cache.data"
        COMMAND jrm "${stage}"
        COMMAND "${CMAKE_COMMAND}" "-DJRM_HEADERS=${generated_headers}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VerifyReflection.cmake"
        COMMAND "${CMAKE_COMMAND}" -E touch ${outputs}
        DEPENDS jrm ${inputs} "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
            "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VerifyReflection.cmake"
        BYPRODUCTS ${byproducts}
        COMMENT "Reflecting ${target} fixtures"
        VERBATIM)
    add_custom_target(${target}_CodeReflector DEPENDS ${outputs})
    add_dependencies(${target} ${target}_CodeReflector)
    foreach(source IN LISTS arg_SOURCES)
        target_sources(${target} PRIVATE "${stage}/${source}")
    endforeach()
    target_include_directories(${target} PRIVATE "${stage}")
endfunction()
