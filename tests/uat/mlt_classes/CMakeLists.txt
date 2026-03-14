
add_executable(RealExample_mlt_classes main.cpp header.h)
target_link_libraries(RealExample_mlt_classes PUBLIC JustReflectMe::Adapter)
target_compile_features(RealExample_mlt_classes PUBLIC ${CXX_STANDARD})

add_jrm_for_current_dir(JRM_mlt_classes)
add_dependencies(RealExample_mlt_classes JRM_mlt_classes)
