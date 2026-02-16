
function(add_jrm_for_current_dir target_name)
    add_custom_target(${target_name}
        COMMAND jrm ${CMAKE_CURRENT_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()

add_subdirectory(classes)
add_subdirectory(enums)