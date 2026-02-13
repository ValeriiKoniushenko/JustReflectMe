
add_custom_target(JRM ALL
   COMMAND jrm ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(RealExample example1.cpp enums.h)
target_link_libraries(RealExample PUBLIC JustReflectMe::Adapter)
add_dependencies(RealExample JRM)