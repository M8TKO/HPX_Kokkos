#!/usr/bin/env bash
set -euo pipefail

TARGET=""
RUN=0

# Parse args
if [[ $# -gt 0 && "$1" == "--run" ]]; then
  RUN=1
  TARGET="$2"
  shift 2
  ARGS=("$@")
else
  ARGS=()
fi

# Clean + build dir
rm -rf build
mkdir build
cd build

# Configure (adapt install paths if needed)
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DKokkos_ROOT=$HOME/kokkos-install \
  -DHPX_DIR=$HOME/hpx-install/lib/cmake/HPX

# Build
cmake --build . -j$(nproc)

# Run if requested
if [[ $RUN -eq 1 ]]; then
  if [[ -z "$TARGET" ]]; then
    echo "Error: No target specified after --run"
    exit 1
  fi
  echo ">>> Running: $TARGET ${ARGS[*]}"
  "./$TARGET" "${ARGS[@]}"
fi
