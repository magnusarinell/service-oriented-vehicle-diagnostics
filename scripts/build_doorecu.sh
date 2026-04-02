#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Add scoop arm-none-eabi toolchain to PATH if not already present
HOME_UNIX=$(cygpath -u "${USERPROFILE:-}" 2>/dev/null || echo "$HOME")
ARM_GCC_BIN="$HOME_UNIX/scoop/apps/gcc-arm-none-eabi/current/bin"
if [[ -d "$ARM_GCC_BIN" && ":$PATH:" != *":$ARM_GCC_BIN:"* ]]; then
  export PATH="$ARM_GCC_BIN:$PATH"
fi

# Add cmake to PATH if not already present
CMAKE_BIN=$(ls -d "$HOME_UNIX/cmake/"*/bin 2>/dev/null | sort -V | tail -1 || true)
if [[ -n "$CMAKE_BIN" && ":$PATH:" != *":$CMAKE_BIN:"* ]]; then
  export PATH="$CMAKE_BIN:$PATH"
fi

cmake -S "$ROOT/DoorECU" \
      -B "$ROOT/DoorECU/build" \
      -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE="$ROOT/DoorECU/toolchain-k64f.cmake"

cmake --build "$ROOT/DoorECU/build"
