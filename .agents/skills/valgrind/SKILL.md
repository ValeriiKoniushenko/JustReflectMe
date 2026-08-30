---
name: valgrind
description: Run JustReflectMe unit tests under Valgrind to find memory errors and definite leaks. Use after memory-related C++ changes, when tests crash or behave nondeterministically, before merging risky lifetime/ownership changes, and when validating the Valgrind CI stage.
---

# Valgrind

Use this skill for dynamic memory checking of the JustReflectMe test suite. It complements the normal build and unit-test checks; it is not a replacement for either one.

## When to run

Run Valgrind when:

- changing ownership, lifetimes, allocation, containers, pointers, or filesystem cleanup;
- adding or modifying tests that create temporary files or directories;
- investigating crashes, use-after-free, uninitialized reads, invalid accesses, or nondeterministic failures;
- validating a change before merging when the affected code is memory-sensitive;
- reproducing or checking the CI Valgrind stage.

For documentation-only, formatting-only, or isolated compile-time changes, the regular build and tests are usually sufficient.

## Prerequisites

Build the test executable first, with debug information and tests enabled:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJRM_DISABLE_TESTS=OFF
cmake --build build -j$(nproc)
```

Confirm that Valgrind is installed and that the executable exists:

```bash
valgrind --version
test -x build/bin/JRMTests
```

If the build directory was configured with a different compiler or stale options, reconfigure it before relying on the result. Use the `clean-build` skill only when an incremental rebuild is not trustworthy.

## Run locally

The project wrapper uses the same options as CI and also keeps Gitea reporting disabled when requested:

```bash
python3 .gitea/check_valgrind.py \
  --executable build/bin/JRMTests \
  --no-gitea \
  --verbose
```

Pass GoogleTest filters after `--` when narrowing an investigation:

```bash
python3 .gitea/check_valgrind.py \
  --executable build/bin/JRMTests \
  --no-gitea \
  -- \
  --gtest_filter=BaseReflectorTests.*
```

The wrapper fails when Valgrind reports errors or definite leaks. It uses exit code `42` internally for Valgrind-detected errors and returns a normal failing process status to the caller. A missing executable returns `2`; a Valgrind startup failure is treated as infrastructure failure.

For a direct diagnostic run without the Gitea wrapper:

```bash
valgrind \
  --leak-check=full \
  --show-leak-kinds=definite \
  --errors-for-leak-kinds=definite \
  --track-origins=yes \
  --error-exitcode=42 \
  build/bin/JRMTests
```

## Interpreting results

- `ERROR SUMMARY: 0 errors` and `definitely lost: 0 bytes` indicate a clean run for the configured checks.
- Fix invalid reads/writes, use-after-free, uninitialized-value reports, and definite leaks in code before suppressing anything.
- Do not suppress a report merely because it originates in a dependency. First determine whether project code triggered the invalid operation or owns the leaked resource.
- If Valgrind cannot start, check the executable architecture, dynamic libraries, container image, and toolchain before changing tests.
- Keep the full verbose output when diagnosing a failure; the CI wrapper may truncate output included in a Gitea review.

## CI alignment

The Gitea workflow runs the equivalent project check in the `valgrind` job:

```bash
python3 .gitea/check_valgrind.py \
  --executable build/bin/JRMTests \
  --verbose
```

CI publishes a replaceable Gitea status and, on failure, a review comment containing the test and Valgrind output. Local runs should use `--no-gitea` unless Gitea reporting is explicitly being tested.

After changing C++ code, run the normal build and unit tests as well as this skill when the change matches the conditions above. Run `clang-format` and the project clang-tidy check before committing.
