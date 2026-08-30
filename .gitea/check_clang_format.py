#!/usr/bin/env python3
"""JustReflectMe entry point for the shared clang-format CI checker."""

from ci.clang_format import main
from jrm_ci import EXCLUDED_DIFF_PATHS


if __name__ == "__main__":
    main(excluded_paths=EXCLUDED_DIFF_PATHS)
