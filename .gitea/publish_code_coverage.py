#!/usr/bin/env python3
"""Publish a replaceable PR review with code-coverage totals and report link."""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path

from gitea_client import GiteaClient, review_marker

CHECK_CONTEXT = "code-coverage"
ATTACHMENT_PREFIX = "ci-code-coverage"


def read_summary(path: Path) -> dict[str, object]:
    try:
        summary = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as error:
        raise RuntimeError(f"could not read coverage summary '{path}': {error}") from error

    required = (
        "line_covered", "line_total", "line_percent",
        "function_covered", "function_total", "function_percent",
        "branch_covered", "branch_total", "branch_percent",
    )
    missing = [key for key in required if key not in summary]
    if missing:
        raise RuntimeError(f"coverage summary is missing: {', '.join(missing)}")
    return summary


def metric(summary: dict[str, object], name: str) -> str:
    covered = int(summary[f"{name}_covered"])
    total = int(summary[f"{name}_total"])
    percent = summary[f"{name}_percent"]
    rendered_percent = "n/a" if percent is None else f"{float(percent):.1f}%"
    return f"{rendered_percent} ({covered}/{total})"


def review_body(summary: dict[str, object], report_url: str | None) -> str:
    report = (
        f"[Open the interactive coverage report]({report_url})"
        if report_url
        else "Interactive report upload was unavailable."
    )
    return "\n".join((
        "## Code coverage",
        "",
        "| Metric | Coverage |",
        "| --- | ---: |",
        f"| Lines | {metric(summary, 'line')} |",
        f"| Functions | {metric(summary, 'function')} |",
        f"| Branches | {metric(summary, 'branch')} |",
        "",
        report,
        "",
        "This report was generated from the pull request's latest CI commit.",
    ))


def publish(summary: dict[str, object], report: Path, *, dry_run: bool) -> None:
    client = GiteaClient.from_env()
    if client is None:
        print("[gitea] client not configured - skipping review publication")
        return

    sha = GiteaClient.resolve_sha() or ""
    pr_number = GiteaClient.resolve_pr_number()
    description = f"{float(summary['line_percent']):.1f}% line coverage"

    if pr_number is None:
        print("[gitea] not a pull_request event - skipping review publication")
        if sha and not dry_run:
            client.publish_check(sha, "success", CHECK_CONTEXT, description)
        return

    if dry_run:
        print(f"[dry-run] would publish code coverage for PR #{pr_number}")
        return

    marker = review_marker(CHECK_CONTEXT)
    try:
        client.dismiss_previous_reviews(pr_number, marker=marker)
        client.delete_issue_attachments(pr_number, name_prefix=ATTACHMENT_PREFIX)
        attachment = client.upload_issue_attachment(
            pr_number,
            str(report),
            name=f"{ATTACHMENT_PREFIX}-{sha[:12] or 'latest'}.html",
        )
        report_url = attachment["browser_download_url"]
        client.create_review(
            pr_number,
            body=review_body(summary, report_url),
            event="COMMENT",
            commit_id=sha,
            marker=marker,
        )
        if sha:
            client.publish_check(
                sha, "success", CHECK_CONTEXT, description, target_url=report_url
            )
    except Exception as error:
        if sha:
            try:
                client.publish_check(sha, "failure", CHECK_CONTEXT, "coverage publication failed")
            except Exception as status_error:
                print(f"[gitea] failed to publish failure status: {status_error}", file=sys.stderr)
        raise RuntimeError(f"failed to publish code coverage: {error}") from error


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--summary", type=Path, required=True, help="gcovr JSON summary")
    parser.add_argument("--report", type=Path, required=True, help="self-contained HTML report")
    parser.add_argument("--dry-run", action="store_true", help="validate inputs without API changes")
    parser.add_argument("--no-gitea", action="store_true", help="print totals without API changes")
    args = parser.parse_args()

    summary = read_summary(args.summary)
    if not args.report.is_file():
        parser.error(f"coverage report does not exist: {args.report}")

    print(f"Code coverage: lines {metric(summary, 'line')}; "
          f"functions {metric(summary, 'function')}; branches {metric(summary, 'branch')}")
    if not args.no_gitea:
        publish(summary, args.report, dry_run=args.dry_run)


if __name__ == "__main__":
    main()
