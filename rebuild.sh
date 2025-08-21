#!/usr/bin/env bash
set -euo pipefail

# --- Defaults / discovery ----------------------------------------------------
TARGET=""
RUN_AFTER_BUILD=0

# Allow overrides via env vars
HPX_DIR_ENV="${HPX_DIR:-$HOME/hpx-install/lib/cmake/HPX}"
KOKKOS_DIR_ENV="${Kokkos_DIR:-}"

if [[ -z "${KOKKOS_DIR_ENV}" ]]; then
  if command -v spack >/dev/null 2>&1; then
    KOKKOS_PREFIX="$(spack location -i kokkos 2>/dev/null || true)"
    if [[ -n "${KOKKOS_PREFIX}" && -d "${KOKKOS_PREFIX}/lib/cmake/Kokkos" ]]; then
      KOKKOS_DIR_ENV="${KOKKOS_PREFIX}/lib/cmake/Kokkos"
    fi
  fi
fi
# Fallback to a manual install if Spack isn’t available
if [[ -z "${KOKKOS_DIR_ENV}" && -d "$HOME/kokkos-install/lib/cmake/Kokkos" ]]; then
  KOKKOS_DIR_ENV="$HOME/kokkos-install/lib/cmake/Kokkos"
fi

if [[ -z "${KOKKOS_DIR_ENV}" ]]; then
  echo "Error: Could not determine Kokkos_DIR."
  echo "Set env var Kokkos_DIR, or install via Spack, or put Kokkos in \$HOME/kokkos-install."
  exit 1
fi

# --- Helper: run a built target ---------------------------------------------
run_target() {
  local tgt="$1"; shift || true
  local args=("$@")
  if [[ -z "$tgt" ]]; then
    echo "Error: no target specified"; exit 1
  fi
  if [[ ! -d build ]]; then
    echo "Error: build/ does not exist. Build first or use --buildrun."; exit 1
  fi
  if [[ ! -x "build/$tgt" ]]; then
    echo "Error: build/$tgt not found or not executable. Build first or use --buildrun."; exit 1
  fi
  echo ">>> Running: build/$tgt ${args[*]}"
  "./build/$tgt" "${args[@]}"
}

# --- Parse args --------------------------------------------------------------
if [[ $# -gt 0 && "$1" == "--run" ]]; then
  if [[ $# -lt 2 ]]; then echo "Usage: $0 --run <target> [args]"; exit 1; fi
  TARGET="$2"; shift 2
  ARGS=("$@")
  run_target "$TARGET" "${ARGS[@]}"
  exit 0
fi

if [[ $# -gt 0 && "$1" == "--buildrun" ]]; then
  if [[ $# -lt 2 ]]; then echo "Usage: $0 --buildrun <target> [args]"; exit 1; fi
  RUN_AFTER_BUILD=1
  TARGET="$2"; shift 2
  ARGS=("$@")
else
  ARGS=()
fi

# --- Clean + configure + build ----------------------------------------------
rm -rf build
mkdir build
cd build

echo ">>> Configuring with:"
echo "    HPX_DIR    = ${HPX_DIR_ENV}"
echo "    Kokkos_DIR = ${KOKKOS_DIR_ENV}"

cmake -S .. -B . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DHPX_DIR="${HPX_DIR_ENV}" \
  -DKokkos_DIR="${KOKKOS_DIR_ENV}"

cmake --build . -j"$(nproc)"

cd ..

# --- Run if requested --------------------------------------------------------
if [[ $RUN_AFTER_BUILD -eq 1 ]]; then
  run_target "$TARGET" "${ARGS[@]}"
fi
