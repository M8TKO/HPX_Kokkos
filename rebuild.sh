#!/usr/bin/env bash
set -euo pipefail

# --- Defaults / simple flags -------------------------------------------------
TARGET=""
RUN_AFTER_BUILD=0

# Optional: allow picking a GPU arch via env var or default to AMPERE86
: "${KOKKOS_GPU_ARCH:=AMPERE86}"

# Optional: CUDA location (set only if not in /usr/local/cuda)
: "${CUDAToolkit_ROOT:=}"

# HPX location (still fine to keep this if you don’t install HPX system-wide)
: "${HPX_DIR:=$HOME/hpx-install/lib/cmake/HPX}"

# --- Helper: run a built target ---------------------------------------------
run_target() {
  local tgt="$1"; shift || true
  local args=("$@")
  if [[ -z "$tgt" ]]; then echo "Error: no target specified"; exit 1; fi
  if [[ ! -d build ]]; then echo "Error: build/ does not exist. Build first or use --buildrun."; exit 1; fi
  if [[ ! -x "build/$tgt" ]]; then echo "Error: build/$tgt not found or not executable. Build first or use --buildrun."; exit 1; fi
  echo ">>> Running: build/$tgt ${args[*]}"
  
  cmake --build build -j"$(nproc)" || { echo "Build failed"; exit 1; }

  if [[ -x "build/$tgt" ]]; then
    echo ">>> Running: build/$tgt ${args[*]}"
    "./build/$tgt" "${args[@]}"
  elif [[ -x "build/bin/$tgt" ]]; then
    echo ">>> Running: build/bin/$tgt ${args[*]}"
    "./build/bin/$tgt" "${args[@]}"
  else
    echo "Error: executable '$tgt' not found in build/ or build/bin/"
    exit 1
  fi
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
echo "    HPX_DIR           = ${HPX_DIR}"
echo "    CUDAToolkit_ROOT  = ${CUDAToolkit_ROOT:-<auto>}"
echo "    Kokkos GPU Arch   = ${KOKKOS_GPU_ARCH}"

# IMPORTANT:
#  - Do NOT pass Kokkos_DIR (we use FetchContent in top-level CMake).
#  - Use the *new* per-arch option -DKokkos_ARCH_<NAME>=ON (e.g., AMPERE86).
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DHPX_DIR="$HOME/hpx-install/lib/cmake/HPX" \
  -DKokkos_ENABLE_CUDA=ON \
  -DKokkos_ENABLE_OPENMP=ON \
  -DKokkos_ENABLE_SERIAL=ON \
  -DKokkos_ARCH_AMPERE86=ON \
  -DDOWNLOAD_EXTRACT_TIMESTAMP=TRUE

cmake --build . -j"$(nproc)"

cd ..

# --- Run if requested --------------------------------------------------------
if [[ $RUN_AFTER_BUILD -eq 1 ]]; then
  run_target "$TARGET" "${ARGS[@]}"
fi
