#!/bin/sh

set -e

MESSAGES="s-messages src/messages.h src/messages.cc"
RELEASE="$1"

[ -z "$RELEASE" ] && { echo "usage: $0 X.Y.Z"; exit 1; }

make clean
make src/messages.h src/messages.cc

git add -f $MESSAGES
git commit -m "Release $RELEASE"
git tag v"$RELEASE"
git archive --prefix=kea-hook-runscript-"$RELEASE"/ -o kea-hook-runscript-"$RELEASE".tar.gz v"$RELEASE"
git rm $MESSAGES
git commit -m "Go back to development after $RELEASE release"

