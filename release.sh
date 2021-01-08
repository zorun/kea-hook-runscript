#!/bin/sh

set -e

RELEASE="$1"

[ -z "$RELEASE" ] && { echo "usage: $0 X.Y.Z"; exit 1; }

# Check repo is clean
git update-index -q --refresh
git diff-files --quiet || { echo "error: ensure git repo has no changes"; exit 1; }

# TODO: regenerate message files to make sure they are up-to-date

git tag v"$RELEASE"
git archive --prefix=kea-hook-runscript-"$RELEASE"/ -o kea-hook-runscript-"$RELEASE".tar.gz v"$RELEASE"
