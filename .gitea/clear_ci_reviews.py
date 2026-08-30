#!/usr/bin/env python3
"""JustReflectMe entry point for shared CI review cleanup."""

from ci.cleanup import main
from jrm_ci import CI_REVIEW_CONTEXTS


if __name__ == "__main__":
    main(
        review_contexts=CI_REVIEW_CONTEXTS,
        attachment_prefixes=("ci-code-coverage",),
    )
