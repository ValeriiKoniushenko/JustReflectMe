add_library(_JustReflectMe_Adapter INTERFACE JustReflectMe/Adapter.h)
add_library(JustReflectMe::Adapter ALIAS _JustReflectMe_Adapter)
target_include_directories(_JustReflectMe_Adapter INTERFACE .)

file(GLOB_RECURSE CPP_SOURCES "JustReflectMe/*.cpp")
set_source_files_properties(${CPP_SOURCES} PROPERTIES LANGUAGE CXX)
file(GLOB_RECURSE HEADERS "*.h")

add_library(_JustReflectMe_Core STATIC ${CPP_SOURCES} ${HEADERS})
add_library(JustReflectMe::Core ALIAS _JustReflectMe_Core)
target_link_libraries(_JustReflectMe_Core PRIVATE
    JustReflectMe::Adapter
    Yaml
    NlohmannJson
)
target_include_directories(_JustReflectMe_Core PUBLIC .)
target_compile_features(_JustReflectMe_Core PUBLIC ${CXX_STANDARD})

if (NOT JRM_DISABLE_TESTS)
    target_compile_definitions(_JustReflectMe_Core PRIVATE JRM_ENABLE_TESTS)
    target_link_libraries(_JustReflectMe_Core PRIVATE gtest)
endif ()

JRFCoreAddCompileOptionsTo(_JustReflectMe_Core)

add_executable(jrm main.cpp)
target_link_directories(jrm PUBLIC .)
target_link_libraries(jrm PRIVATE
    JustReflectMe::Adapter
    JustReflectMe::Core
)
JRFCoreAddCompileOptionsTo(jrm)
