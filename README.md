# Just Reflect Me - JRM

Fast & user-friendly reflection application to reflect the C++ code. Everything that you need is to connect this library to your project (as sources or just a binary) and set up a few things in CMake post-build actions.

## Technical details

- Written in C++23
- CMake-based build system
- No dependencies (optionally Google Test)

## CMake macros

Put `set(JRM_DISABLE_TESTS ON)` before connecting this library to disable tests. It will disable all levels of working with unit tests inside this library to speed up your build.