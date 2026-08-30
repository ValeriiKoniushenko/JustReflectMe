#!/usr/bin/env sh
set -eu

# tag::clone_and_build[]
git clone --recurse-submodules https://github.com/ValeriiKoniushenko/JustReflectMe.git
cd JustReflectMe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJRM_DISABLE_TESTS=OFF
cmake --build build --parallel
# end::clone_and_build[]

# tag::run_generator[]
build/bin/jrm /absolute/path/to/your/project
# end::run_generator[]

# tag::cli_info[]
build/bin/jrm --help
build/bin/jrm --version
build/bin/jrm --fallback-config
# end::cli_info[]

# tag::run_tests[]
build/bin/JRMTests
# end::run_tests[]

# tag::format_check[]
clang-format --dry-run --Werror \
  docs/modules/ROOT/examples/quickstart/main.cpp \
  docs/modules/ROOT/examples/quickstart/model.h
# end::format_check[]

# tag::benchmark[]
cmake -S . -B build/benchmark -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DJRM_DISABLE_TESTS=ON -DJRM_ENABLE_BENCHMARKS=ON
cmake --build build/benchmark --parallel
build/benchmark/bin/JRMStringAndNavigationBenchmarks
# end::benchmark[]

# tag::coverage[]
cmake -S . -B build/coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DJRM_DISABLE_TESTS=OFF \
  -DJRM_ENABLE_CODE_COVERAGE=ON \
  -DJRM_GENERATE_CODE_COVERAGE_HTML=ON
cmake --build build/coverage --target JRMCodeCoverageHtml --parallel
# end::coverage[]

# tag::valgrind[]
python3 .gitea/check_valgrind.py \
  --executable build/bin/JRMTests \
  --no-gitea \
  --verbose
# end::valgrind[]

# tag::docs_site[]
cd docs
npx antora --fetch antora-playbook.yml
# end::docs_site[]
