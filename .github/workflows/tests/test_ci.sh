#!/bin/sh

set -e

########################################
# Note: run this from the repo root.
########################################

# Tests
act pull_request -W .github/workflows/tests.yml \
    -e .github/workflows/tests/pr_payload.json

# Docs
act push -W .github/workflows/docs.yml \
    -e .github/workflows/tests/pr_payload.json

# Release
act release -W .github/workflows/release_img.yml \
    -e .github/workflows/tests/push_payload.json
