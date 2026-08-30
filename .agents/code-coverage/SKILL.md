---
name: code-coverage
description: Generate the JustReflectMe HTML code coverage report with the configured GCC or Clang build.
---

# Code Coverage

Generate source coverage through the `JRMCodeCoverageHtml` CMake target. It runs `JRMTests` and writes an HTML report below the active build directory. The report covers `sources/`; dependency code is excluded.

## Preconditions

- Coverage supports GCC and Clang only. MSVC is not supported.
- Tests must be enabled: `JRM_DISABLE_TESTS=OFF`.
- Both options must be enabled at CMake configure time:
  - `JRM_ENABLE_CODE_COVERAGE=ON`
  - `JRM_GENERATE_CODE_COVERAGE_HTML=ON`
- GCC requires `gcov` and a `gcovr` version supporting `--exclude-throw-branches` and `--exclude-unreachable-branches`.
- Clang requires `llvm-profdata` and `llvm-cov` from the selected Clang toolchain.

Coverage flags affect compilation. Prefer a dedicated coverage build directory, or explicitly accept the recompilation of an existing Debug tree when enabling coverage.

## Generate The Report

First inspect the selected build directory when its configuration is unknown:

```sh
cmake -N -L build/gcc/debug | rg 'JRM_(ENABLE_CODE_COVERAGE|GENERATE_CODE_COVERAGE_HTML|DISABLE_TESTS|CODE_COVERAGE_OUTPUT_DIRECTORY)'
```

For the repository's active GCC Debug configuration, generate the report directly:

```sh
cmake --build build/gcc/debug --target JRMCodeCoverageHtml -j16
```

The default report is `build/gcc/debug/coverage/index.html`. Confirm it was produced before reporting success:

```sh
test -f build/gcc/debug/coverage/index.html
```

`JRMCodeCoverageHtml` recreates its configured coverage output directory on every run. Do not run it concurrently against the same build directory.

## Configure A Coverage Build

When coverage is not enabled, configure or reconfigure the intended build directory before invoking the target:

```sh
cmake -S . -B build/gcc/debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DJRM_DISABLE_TESTS=OFF \
  -DJRM_ENABLE_CODE_COVERAGE=ON \
  -DJRM_GENERATE_CODE_COVERAGE_HTML=ON
cmake --build build/gcc/debug --target JRMCodeCoverageHtml -j16
```

The report output directory defaults to `<build-dir>/coverage`. It may be customized with `JRM_CODE_COVERAGE_OUTPUT_DIRECTORY`, but it must remain a child of the CMake binary directory.

## Clang

Use a build directory configured with `clang++`; a directory named `clang` is not sufficient evidence of the active compiler. Configure the coverage options as above with `-DCMAKE_CXX_COMPILER=clang++`, then run the same `JRMCodeCoverageHtml` target. The Clang report is written to `<build-dir>/coverage/index.html` and uses `llvm-profdata` plus `llvm-cov` internally.

## Troubleshooting

- If CMake says the report target does not exist, enable both coverage options and reconfigure.
- If CMake rejects the configuration because tests are disabled, reconfigure with `-DJRM_DISABLE_TESTS=OFF`.
- If a tool is missing, install or select the matching GCC/Clang coverage toolchain, then reconfigure so CMake can discover it.
- If changing compilers or coverage instrumentation on an existing build creates inconsistent artifacts, use the `clean-build` skill for a fresh build directory before retrying.
