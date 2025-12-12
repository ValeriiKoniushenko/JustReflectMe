add_library(_JustReflectMe_Adapter INTERFACE)
add_library(JustReflectMe::Adapter ALIAS _JustReflectMe_Adapter)

target_compile_definitions(_JustReflectMe_Adapter INTERFACE CLASS)
#target_compile_definitions(_JustReflectMe_Adapter PUBLIC PROPERTY)


file(GLOB_RECURSE CPP_SOURCES "JustReflectMe/*.cpp")
set_source_files_properties(${CPP_SOURCES} PROPERTIES LANGUAGE CXX)
file(GLOB_RECURSE HEADERS "*.h")

add_library(_JustReflectMe_Core STATIC ${CPP_SOURCES} ${HEADERS})
add_library(JustReflectMe::Core ALIAS _JustReflectMe_Core)
target_link_libraries(_JustReflectMe_Core PRIVATE
    JustReflectMe::Adapter
)

add_executable(JustReflectMe main.cpp)
target_link_directories(JustReflectMe PUBLIC .)
target_link_libraries(JustReflectMe PRIVATE
    JustReflectMe::Adapter
    JustReflectMe::Core
)