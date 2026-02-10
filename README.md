# Just Reflect Me - JRM

Fast & user-friendly reflection application to reflect the C++ code. Everything that you need is to connect this library to your project (as sources or just a binary) and set up a few things in CMake post-build actions.

## Technical details

- Written in C++23
- CMake-based build system
- No dependencies (optionally Google Test)

## CMake macros

Put `set(JRM_DISABLE_TESTS ON)` before connecting this library to disable tests. It will disable all levels of working with unit tests inside this library to speed up your build.

## Integration

1. Clone this repository to your project's root directory.
2. Using CMake add this as a subdirectory: `add_subdirectory(path/to/JRM)`.
3. Connect the adapter to your CMake's target: `target_link_libraries(YourTarget PUBLIC JustReflectMe::Adapter)`
4. Add auto-run of the code generator for your main target:
   ```cmake
   add_custom_target(JRM ALL
      COMMAND jrm ${CMAKE_SOURCE_DIR}
   )
   add_dependencies(YourTargetHere JRM)
   ```
That's it!

---

**Builds**:

- [![MSVC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Debug%2F&label=MSVC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Debug/) [![MSVC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Release%2F&label=MSVC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Release/)
- [![GCC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Debug%2F&label=GCC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Debug/) [![GCC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Release%2F&label=GCC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Release/)
- [![Clang Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Debug%2F&label=Clang%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Debug/) [![Clang Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Release%2F&label=Clang%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Release/)


