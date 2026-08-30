#!/usr/bin/env python3
"""JustReflectMe entry point for the shared clang-tidy CI checker."""

from ci.clang_tidy import main
from jrm_ci import EXCLUDED_DIFF_PATHS


if __name__ == "__main__":
    main(excluded_paths=EXCLUDED_DIFF_PATHS)
