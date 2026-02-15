
add_executable(RealExample_classes main.cpp header.h)
target_link_libraries(RealExample_classes PUBLIC JustReflectMe::Adapter)

# add_dependencies(RealExample_classes JRM)
