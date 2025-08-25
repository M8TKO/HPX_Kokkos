cmake_minimum_required(VERSION 3.20)
project(HPX_KokkosWithCuda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # compiler must support C++17
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

# ---- Dependencies (host side) ----
# ---- Dependencies (host side) ----
find_package(OpenMP REQUIRED)

# HPX
set(HPX_DIR "$ENV{HOME}/hpx-install/lib/cmake/HPX" CACHE PATH "HPX cmake dir")
find_package(HPX REQUIRED CONFIG)

# ------------------ Kokkos from Git (FetchContent) ------------------
include(FetchContent)

# Choose CUDA arch here (example: 86 for Ampere, 80 for A100, 90 for Hopper, etc.)
# You can add multiple arch values comma-separated like "80,86"
#set(KOKKOS_ARCH "AMPERE86" CACHE STRING "Kokkos GPU architecture")
set(CMAKE_CUDA_ARCHITECTURES 86)
FetchContent_Declare(
  Kokkos
  URL https://github.com/kokkos/kokkos/archive/refs/tags/4.5.01.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Important: configure Kokkos once, at the top-level, with CUDA + OpenMP.
# Do NOT install it system-wide; just build it in the build tree.
#set(Kokkos_ENABLE_OPENMP ON CACHE BOOL "")
#set(Kokkos_ENABLE_CUDA   ON CACHE BOOL "")
#set(Kokkos_ENABLE_SERIAL ON CACHE BOOL "")           # harmless, keeps host path available
#set(Kokkos_ENABLE_LIBDL  OFF CACHE BOOL "")          # avoid libdl noise
#set(Kokkos_ARCH_${KOKKOS_CUDA_ARCH} ON CACHE BOOL "")# translates to Kokkos_ARCH_86=ON etc.
#set(Kokkos_CXX_STANDARD 17 CACHE STRING "")

# If your CUDA toolkit is not on PATH/LD_LIBRARY_PATH, also point CMake at it:
set(CUDAToolkit_ROOT "/usr/local/cuda-12" CACHE PATH "")

FetchContent_MakeAvailable(Kokkos)

# ---- Subproject: CUDA/Kokkos implementation ----
# (We’ll fix gpu/ CMake in the next step to build CUDA objects properly.)
add_subdirectory(gpu)

# ---- Main executable (HPX side, host compiler) ----
add_executable(HPX_KokkosWithCuda
  src/hpx.cpp
)

# Include paths for comm.hpp and Kokkos headers (headers only; do NOT link Kokkos here)
target_include_directories(HPX_KokkosWithCuda PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  $<TARGET_PROPERTY:Kokkos::kokkos,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(HPX_KokkosWithCuda
  PRIVATE
    kokkos_cuda_objs
    HPX::hpx
    HPX::iostreams_component
    OpenMP::OpenMP_CXX
)


# Pick the right HPX main/init wrapper
if(TARGET HPX::wrap_main)
  target_link_libraries(HPX_KokkosWithCuda PRIVATE HPX::wrap_main)
elseif(TARGET HPX::hpx_init)
  target_link_libraries(HPX_KokkosWithCuda PRIVATE HPX::hpx_init)
elseif(TARGET HPX::hpx_main)
  target_link_libraries(HPX_KokkosWithCuda PRIVATE HPX::hpx_main)
else()
  message(FATAL_ERROR "No suitable HPX main/wrapper target found")
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
  src/hpx.cpp
)
