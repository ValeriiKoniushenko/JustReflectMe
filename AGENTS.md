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

- In general, it was planned to generate the `.h` & `.cpp` files. But it was decided to generate only the `.h` files, in
  the format of `.inl` files. It means that the generated content must be included inside the target `.h` file by means
  of the `#include` directive at the end of the target `.h` file.
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

Shared skills in `.agents/Agents/skills/` must read this section before running a
project-specific command. CMake creates ignored relative links for them in
`.agents/skills/`; project-local skills may coexist there.

- Default configure command: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJRM_DISABLE_TESTS=OFF`.
  The standard incremental command is `cmake --build build --parallel`. Build artifacts
  are under `build/bin/`, including `jrm` and `JRMTests`.
- Run the full test suite with `cmake --build build --target JRMTests --parallel` followed
  by `build/bin/JRMTests`, or `ctest --test-dir build --output-on-failure` when CTest is
  configured.
- Benchmarks use `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DJRM_ENABLE_BENCHMARKS=ON`
  and `build/bin/JRMStringAndNavigationBenchmarks`. List them with
  `--benchmark_list_tests`; focus with `--benchmark_filter=<regex>`.
- Coverage supports GCC and Clang, requires `JRM_DISABLE_TESTS=OFF`,
  `JRM_ENABLE_CODE_COVERAGE=ON`, and `JRM_GENERATE_CODE_COVERAGE_HTML=ON`. Build
  `JRMCodeCoverageHtml`; the default report is `build/coverage/index.html`.
- For Valgrind, configure a Debug build with tests enabled and run
  `python3 .gitea/check_valgrind.py --executable build/bin/JRMTests --no-gitea --verbose`.
  Pass GoogleTest filters after `--` when investigating a subset.
- Run `python3 .gitea/check_clang_format.py --no-gitea` to check changed C++ formatting;
  add `--fix` to apply formatting. Run
  `python3 .gitea/check_clang_tidy.py --build-dir build --no-gitea` for changed-file
  clang-tidy analysis. These helpers accept `--files` for a focused file list.
- Documentation sources use Antora under `docs/modules/ROOT/`; never read or edit
  generated `docs/html/`. Update `docs/modules/ROOT/nav.adoc` when page paths change.
  The normal site build is `cd docs && npx antora --fetch antora-playbook.yml`.

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
