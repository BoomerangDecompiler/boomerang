#!/bin/bash
set -e

git clean -fdx src
./scripts/run-clang-format.py -r src -e *machO/objc* | git apply -p0 &>/dev/null && git commit -a --amend --no-edit || true
./scripts/check-commit.sh
