
add_executable(RealExample_classes main.cpp header.h)
target_link_libraries(RealExample_classes PUBLIC JustReflectMe::Adapter)
target_compile_features(RealExample_classes PUBLIC ${CXX_STANDARD})

# add_dependencies(RealExample_classes JRM)
