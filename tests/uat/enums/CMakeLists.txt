
add_executable(RealExample_enums main.cpp header.h)
target_link_libraries(RealExample_enums PUBLIC JustReflectMe::Adapter)

# add_dependencies(RealExample_enums JRM)
