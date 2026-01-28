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
   add_custom_command(
    TARGET Draft
    PRE_BUILD
    COMMAND jrm ${CMAKE_SOURCE_DIR}
   )
   ```
That's it!
