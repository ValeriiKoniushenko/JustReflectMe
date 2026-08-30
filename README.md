# JustReflectMe (JRM)

Build-time reflection and code generation for C++ classes, structs, fields, and scoped enums.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](CMakeLists.txt)
[![CMake](https://img.shields.io/badge/build-CMake-informational.svg)](CMakeLists.txt)

JRM scans explicitly annotated headers and generates ordinary `R<T>` specializations in adjacent
`*.generated.h` files. Applications use those specializations for type and field metadata, scoped
enum conversion, runtime field lookup, and JSON serialization—without compiler patches.

## What is included

- `CLASS()`, `FIELD()`, `R_FRIEND()`, and `ENUM_CLASS()` opt-in markers.
- Generated class/struct and enum-class metadata.
- A resource-stream serialization API with `nlohmann::json` as the built-in adapter.
- Project-local YAML configuration and timestamp-based incremental generation.
- CMake targets for the runtime adapter, generator, tests, coverage, and microbenchmarks.
- GCC, Clang, and MSVC project support.

The shipped CMake targets currently require C++23. The project policy is moving toward C++26 as
cross-compiler support becomes available.

## Documentation

The essential Antora documentation is in [`docs/modules/ROOT/pages`](docs/modules/ROOT/pages) and
starts at the [JustReflectMe overview](docs/modules/ROOT/pages/index.adoc).

- [Getting started](docs/modules/ROOT/pages/getting-started.adoc)
- [Class reflection](docs/modules/ROOT/pages/class-reflection.adoc)
- [Enum reflection](docs/modules/ROOT/pages/enum-reflection.adoc)
- [Serialization](docs/modules/ROOT/pages/serialization.adoc)
- [Generated API reference](docs/modules/ROOT/pages/generated-api.adoc)
- [Configuration reference](docs/modules/ROOT/pages/configuration.adoc)
- [Scope and limitations](docs/modules/ROOT/pages/limitations.adoc)
- [Build, tests, coverage, and benchmarks](docs/modules/ROOT/pages/development.adoc)

The navigation for the complete documentation set is
[`docs/modules/ROOT/nav.adoc`](docs/modules/ROOT/nav.adoc). Build the site separately with Antora
using [`docs/antora-playbook.yml`](docs/antora-playbook.yml); generated site output is not a source
of truth.

## Important boundary

JRM is a focused source scanner, not a complete C++ frontend. Analysis and reflected-type
cross-linking are currently file-local, template-scope reflection is unsupported, and conventional
declaration formatting is recommended. Read the limitations page before adopting JRM for a
template-heavy or macro-generated model.

## License and contribution

JRM is available under the [MIT License](LICENSE). See [CONTRIBUTING.md](CONTRIBUTING.md),
[CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md) before contributing or
reporting a vulnerability.

