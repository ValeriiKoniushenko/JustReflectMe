
add_executable(RealExample_enums main.cpp header.h)
target_link_libraries(RealExample_enums PUBLIC JustReflectMe::Adapter)
target_compile_features(RealExample_enums PUBLIC ${CXX_STANDARD})

# add_dependencies(RealExample_enums JRM)
