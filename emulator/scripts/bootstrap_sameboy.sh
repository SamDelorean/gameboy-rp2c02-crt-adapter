#!/usr/bin/env sh
set -eu

# SameBoy commit selected for the first hybrid-emulator integration pass.
SAMEBOY_COMMIT="213a12ce93d66b105a113debd9396306066a7cfc"

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
EMU_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
THIRD_PARTY_DIR="$EMU_DIR/third_party"
SAMEBOY_DIR="$THIRD_PARTY_DIR/SameBoy"

mkdir -p "$THIRD_PARTY_DIR"

if [ ! -d "$SAMEBOY_DIR/.git" ]; then
    git clone https://github.com/LIJI32/SameBoy.git "$SAMEBOY_DIR"
fi

cd "$SAMEBOY_DIR"
git fetch origin "$SAMEBOY_COMMIT"
git checkout --detach "$SAMEBOY_COMMIT"

# We only need the reusable core archive plus the checked-in Core headers.
# Building SameBoy's full `lib` target also generates processed public headers
# with the external `cppp` utility; that extra tool is unnecessary here.
make build/lib/libsameboy.a \
    CONF=release \
    DISABLE_DEBUGGER=1 \
    DISABLE_CHEATS=1 \
    DISABLE_CHEAT_SEARCH=1 \
    DISABLE_REWIND=1

printf '\nSameBoy static core ready at:\n  %s\n' "$SAMEBOY_DIR"
printf '\nConfigure the hybrid emulator with:\n'
printf '  cmake -S "%s" -B "%s/build" -DGBCRT_ENABLE_SAMEBOY=ON -DSAMEBOY_ROOT="%s"\n' \
    "$EMU_DIR" "$EMU_DIR" "$SAMEBOY_DIR"
