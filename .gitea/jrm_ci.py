"""JustReflectMe-specific CI policy."""

EXCLUDED_DIFF_PATHS = (
    "dependencies/",
    "docs/",
    "cmake/",
    "data/",
)

CI_REVIEW_CONTEXTS = (
    "clang-format",
    "clang-tidy",
    "code-coverage",
    "valgrind",
)
