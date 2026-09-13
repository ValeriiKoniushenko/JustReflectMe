add_library(_JustReflectMe_Adapter STATIC JustReflectMe/Adapter.cpp JustReflectMe/Adapter.h)
add_library(JustReflectMe::Adapter ALIAS _JustReflectMe_Adapter)
target_link_libraries(_JustReflectMe_Adapter PUBLIC NlohmannJson)
target_include_directories(_JustReflectMe_Adapter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
JRMApplyTargetSettings(_JustReflectMe_Adapter)

add_library(_JustReflectMe_Core STATIC
    JustReflectMe/Cache.cpp JustReflectMe/Cache.h
    JustReflectMe/Config.cpp JustReflectMe/Config.h
    JustReflectMe/Enums.cpp JustReflectMe/Enums.h
    JustReflectMe/FileData.cpp JustReflectMe/FileData.h
    JustReflectMe/FileNavigationHelper.cpp JustReflectMe/FileNavigationHelper.h
    JustReflectMe/FileProcessor.cpp JustReflectMe/FileProcessor.h
    JustReflectMe/JustReflectMe.cpp JustReflectMe/JustReflectMe.h
    JustReflectMe/Scopes.cpp JustReflectMe/Scopes.h
    JustReflectMe/StringHelper.cpp JustReflectMe/StringHelper.h
    JustReflectMe/Reflectors/BaseReflector.cpp JustReflectMe/Reflectors/BaseReflector.h
    JustReflectMe/Reflectors/ClassReflector.cpp JustReflectMe/Reflectors/ClassReflector.h
    JustReflectMe/Reflectors/EnumClassReflector.cpp JustReflectMe/Reflectors/EnumClassReflector.h
)
add_library(JustReflectMe::Core ALIAS _JustReflectMe_Core)
target_link_libraries(_JustReflectMe_Core PRIVATE JustReflectMe::Adapter Yaml PUBLIC NlohmannJson)
target_include_directories(_JustReflectMe_Core
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${PROJECT_BINARY_DIR}/include")
JRMApplyTargetSettings(_JustReflectMe_Core)

add_executable(jrm main.cpp)
target_link_libraries(jrm PRIVATE JustReflectMe::Core)
JRMApplyTargetSettings(jrm)
