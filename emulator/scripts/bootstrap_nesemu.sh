#!/bin/sh
set -eu

REVISION=4966aa09259ef965d4b6bd2635a1dfe57a8569cb
REPOSITORY=https://github.com/johnmph/NESEmu.git
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
DEST="$ROOT/third_party/NESEmu"

mkdir -p "$ROOT/third_party"

if [ ! -d "$DEST/.git" ]; then
    rm -rf "$DEST"
    git init -q "$DEST"
    git -C "$DEST" remote add origin "$REPOSITORY"
fi

if ! git -C "$DEST" cat-file -e "$REVISION^{commit}" 2>/dev/null; then
    git -C "$DEST" fetch --quiet --depth 1 origin "$REVISION"
fi

git -C "$DEST" checkout --quiet --detach "$REVISION"

echo "NESEmu PPU pinned at $(git -C "$DEST" rev-parse HEAD)"
