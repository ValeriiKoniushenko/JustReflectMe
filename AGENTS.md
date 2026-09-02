# AGENTS.md

## Project

JustReflectMe (jrm) — a code reflector library for C++ sources.

## Architecture

- It scans the project by passing the corresponding command line arguments. E.g.: `jrm <path_to_project_root>`
- After that it scans or spawns the jrm config by the next path: `<path_to_project_root>/.jrm`
    - It contains two things: `cache.data` and `config.yaml`
    - `cache.data` is a file with the last updated timestamps of all the files that were reflected.
    - `config.yaml` is a file with settings for the jrm.
- After that, it scans file by file (`JustReflectMe::goThroughFiles`), and right now it has only the context only per
  file. So, it can't interfare the logic or the types between different files and corresponding contexts/types.
- Next, the `class FileProcessor` will scan the file for C++ context. One file → one `FileProcessor` instance. We can
  find this code here: `JustReflectMe.cpp` -> `JustReflectMe::goThroughFiles`
    - To set up the `FileProcessor` instance, we must set up C++ Reflectors: Class Reflector, Enum Class Reflector,
      etc. - using `FileProcessor::registerReflector` method.
    - After that, we can call `FileProcessor::run` method to run the whole file-reflecting process.
        - The next step is getting of the file content with some clearings (`FileProcessor::getFileContent`). E.g.,
          removing of all comments, resolving of double, single quotes, etc.
        - After that, we will scan the whole file content using the registered reflectors in the method:
          `FileProcessor::processContent`. It has two main logics: take one reflector and scan the content
          (`BaseReflector::scanContent`), and the second is the same after scanning by all reflectors, but taking it
          again from the first reflector will call `BaseReflector::postScanCrossLinksResolving` (it was added to resolve
          some intersection data per file). Next, we'll find information about per-reflector logic:
            - Every reflector works by virtue of finding the correct 'trigger word' in the file content. E.g., for class
              reflector it's `CLASS()` before all class definitions; for enum class reflector it's `ENUM_CLASS()` before
              all enum class definitions in the source code. Finally, every reflector will find the start of every
              'trigger word' entrance (just a pointer to the start of the 'trigger word' in the file content).
            - The next step is scanning of the file scopes (`Scopes::scan`): file scope, namespace scopes, class scopes,
              etc. With getting of the file content. This data about the scanned scopes will be stored inside the
              `class FileData`, not inside some specific reflector.
            - And after that will be called pure virtual method `BaseReflector::onScan`. The main logic of all
              reflectors for C++ context is described there, inside this overridden method. All next logic inside this
              overridden method is connected to some specific domain of the C++ language; for classes it's described
              into `ClassReflector::onScan`, for enum classes `EnumClassReflector::onScan`.
        - And when all scans are passed by all reflectors in the `FileProcessor`, it can start generating a new
          `.generated.*` files, new generated code. It will call this function for that:
          `FileProcessor::generateNewContent`. Simply speaking, it will ask every reflector for its specific data and
          will populate it together to the one final file.

### Notes

- JRM currently generates only sibling `*.generated.h` files. The generator appends the corresponding `#include`
  directive to the original header. Generated `.cpp` and `.inl` files are not part of the current end-to-end path.
- At the start of development, the possible Serialization/Deserialization format was designed to be format-agnostic. But
  by the development it's stuck to the JSON format. So, now for serialization/deserialization we must use
  `class RJsonResourceStream` as a format adapter. The final thing that executes data pushing/pulling from the object
  looks like this: `RResourceStream<RJsonResourceStream>`. You can find the smallets example of it in the unit tests:
  `TEST(Enums, Serialize)`, `TEST(Enums, Deserialize)`; for classes: `TEST(Classes, Serialize_Animal)`,
  `TEST(Classes, Deserialize_Animal)`.

## Build

- CMake, cross-platform (Linux, Windows): MSVC / GCC / Clang.
- MSVC debug info: use `/Z7`, not `/Zi` — required for ccache compatibility.
- CI runs on self-hosted Gitea (gitea.vakon.dev) via act_runner:
  clang-format, clang-tidy, build (GCC+Clang), unit tests, valgrind.

## Agent Skill Workflows

The skills linked under `.agents/skills/` are generalized workflows. This section is the
project-specific source of truth for every linked skill. The links are supplied by the
`.agents/Agents` submodule; do not edit `.agents/Agents/`. Project-local skills may coexist
under `.agents/skills/`.

If required submodules are absent, initialize the pinned revisions before configuring:
`git submodule sync --recursive` followed by `git submodule update --init --recursive`.
Do not update submodules to newer remote revisions as part of a build or verification run.

### `build`

- The default development build directory is `build/`. If it is not configured, run
  `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJRM_DISABLE_TESTS=OFF`.
  CMake exports `build/compile_commands.json` automatically.
- Run the standard incremental build with `cmake --build build --parallel`.
- Runtime artifacts are under `build/bin/`: `jrm` is the generator and `JRMTests` is the
  unit-test executable when tests are enabled. Libraries are under `build/lib/`.

### `clean-build`

- Use a clean build only after structural CMake or toolchain changes, when stale cache state
  is demonstrated, or when an incremental build fails inexplicably.
- For the default layout, remove only `build/`, then run the default configure and incremental
  build commands above. When working in a dedicated benchmark or coverage directory, remove
  only that selected binary directory.
- Preserve sources, tests, benchmarks, documentation, `.jrm` state, and submodule contents.

### `test`

- Tests require `JRM_DISABLE_TESTS=OFF`. Build them with
  `cmake --build build --target JRMTests --parallel`, then run the complete suite with
  `build/bin/JRMTests`.
- CTest is not currently registered by this project; invoke `JRMTests` directly.
- For focused iteration, pass a GoogleTest filter such as
  `build/bin/JRMTests --gtest_filter='Classes.*'`. Run the unfiltered suite before declaring
  a behavior change verified.
- Building `JRMTests` first runs the `JRMTests_CodeReflector` target, which invokes `jrm` on
  `tests/` so the user-acceptance headers compile against freshly generated specializations.

### `benchmark`

- Use a dedicated Release build:
  `cmake -S . -B build/benchmark -G Ninja -DCMAKE_BUILD_TYPE=Release -DJRM_DISABLE_TESTS=ON -DJRM_ENABLE_BENCHMARKS=ON`,
  followed by `cmake --build build/benchmark --parallel`.
- The executable is `build/benchmark/bin/JRMStringAndNavigationBenchmarks`. Discover cases with
  `--benchmark_list_tests` and focus with `--benchmark_filter='<regex>'`.
- For comparisons, use repeated runs on the same machine and configuration. These are
  `StringHelper` and `FileNavigationHelper` microbenchmarks, not end-to-end project-scan latency.
- Coverage and benchmarks cannot be enabled in the same CMake configuration.

### `ecs-benchmarks`

- This repository has no ECS subsystem, ECS benchmark source, or ECS benchmark target. The
  `ecs-benchmarks` skill is currently not applicable. Report that absence explicitly; do not
  present `JRMStringAndNavigationBenchmarks` as an ECS suite.

### `code-coverage`

- Use a dedicated Debug build and do not run two coverage jobs against the same binary directory:
  `cmake -S . -B build/coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJRM_DISABLE_TESTS=OFF -DJRM_ENABLE_CODE_COVERAGE=ON -DJRM_GENERATE_CODE_COVERAGE_HTML=ON`.
- Generate the report with
  `cmake --build build/coverage --target JRMCodeCoverageHtml --parallel`. With that dedicated
  layout, the default HTML entry point is `build/coverage/coverage/index.html`; GCC also writes
  `build/coverage/coverage/summary.json`.
- Only GCC and Clang are supported. GCC requires `gcov` and a `gcovr` version supporting
  `--exclude-throw-branches` and `--exclude-unreachable-branches`; Clang requires compatible
  `llvm-profdata` and `llvm-cov` executables.
- `JRM_CODE_COVERAGE_OUTPUT_DIRECTORY` may override the report location, but it must be a proper
  child of the active CMake binary directory.

### `codegen`

- Regenerate when reflection markers or reflected declarations change; reflector, configuration,
  or generation logic changes; generated headers are missing or stale; or cache behavior is under
  test. Build `jrm` first, then run `build/bin/jrm <absolute-project-root>`.
- JRM loads or creates `<project-root>/.jrm/config.yaml`, tracks successful inputs in
  `<project-root>/.jrm/cache.data`, writes sibling `*.generated.h` files, and adds their include to
  the original headers. Never hand-edit generated headers.
- To force a scan, delete only the target project's `.jrm/cache.data` or temporarily set
  `alwaysDirtyCache: true`; preserve `config.yaml` unless configuration reset is intentional.
- For this repository's unit-test fixtures, building `JRMTests` performs code generation
  automatically. Follow explicit regeneration with the standard incremental build and report
  generated changes separately.
- CMake also configures `sources/JustReflectMe/version.h` from `version.h.template`; rerun CMake
  configure when the project version or that template changes.

### `docs-generation`

- Antora sources are under `docs/modules/ROOT/`: pages in `pages/`, live tagged examples in
  `examples/`, and navigation in `nav.adoc`. Update `nav.adoc` whenever a page is added, removed,
  or renamed.
- Never read, edit, or use generated `docs/html/` as a source. Do not paste real source or test
  code into pages; tag the authoritative file and include it through Antora/AsciiDoc instead.
- After every documentation change, build the site from `docs/` with
  `npx antora --fetch antora-playbook.yml` and report any missing Node/Antora prerequisite or
  download failure.

### `lint-and-format`

- Check changed C++ formatting with
  `python3 .gitea/check_clang_format.py --no-gitea`; add `--fix` only when formatting changes are
  requested. Use `--files <paths...>` for an explicit focused set.
- Run changed-file static analysis with
  `python3 .gitea/check_clang_tidy.py --build-dir build --no-gitea`. Configure `build/` with tests
  enabled first so `compile_commands.json` covers production and test sources. The helper also
  accepts `--files <paths...>`.
- These wrappers mirror the repository's CI exclusions. Do not introduce new clang-tidy
  suppressions without a documented reason.

### `valgrind`

- Configure a Debug build with `JRM_DISABLE_TESTS=OFF`, build `JRMTests`, and confirm Valgrind is
  installed.
- Run the CI-aligned local wrapper with
  `python3 .gitea/check_valgrind.py --executable build/bin/JRMTests --no-gitea --verbose`.
  Pass focused GoogleTest arguments after `--`, for example
  `-- --gtest_filter='Classes.*'`; run the full suite for final memory verification.
- Invalid accesses, uninitialized reads, Valgrind errors, and definite leaks fail the check. The
  wrapper maps those failures to a nonzero exit and distinguishes missing-tool startup failures.

### `verification-run`

- Build first with the `build` workflow. For C++ behavior changes, also run the complete `test`
  workflow. Documentation-only changes use the documentation site build instead of unit tests;
  performance, coverage, and memory claims additionally use their corresponding skills.
- For generator, CLI, or end-to-end pipeline changes, run `build/bin/jrm` against a temporary copy
  of `docs/modules/ROOT/examples/quickstart/`, with a 60-second timeout. The smoke check passes only
  if the command exits `0`, reports `Ended SUCCESSFULLY`, and creates a nonempty
  `model.generated.h` in the temporary project. Do not run this smoke check directly in the
  documentation source directory because generation modifies the scanned project.
- A change is verified only when every required command exits successfully. Report skipped checks,
  unavailable tools, timeouts, and failed pass criteria explicitly.

## Conventions

- Target standard: **C++26** (`-std=c++2c` on GCC/Clang). Support is still partial across compilers — verify a given
  feature is actually available on the toolchain/compiler version in use before relying on it, rather than assuming full
  C++26 conformance.
- **`dependencies/` folder**: these are separate real repos, not engine code. Generally out of scope for engine-side
  changes; exceptions are
  `Utils` and `JustReflectMe`, where cross-cutting fixes may be relevant.
- **Testing**: TDD-leaning — add at least minimal tests alongside the new implementation, not after. Tests live in
  `tests/`.
- Prefer `std::array` over C arrays (mid-migration — match existing style in the file you're editing).
- clang-tidy must pass; don't introduce new suppressions without reason.

## Verification

- After implementing a change, verify it before considering it done:
    1. **Build** — use the `build` skill to compile and confirm there are no build errors/warnings introduced.
- Don't treat a change as complete on "looks correct" alone — actually build and run it via these skills first.
