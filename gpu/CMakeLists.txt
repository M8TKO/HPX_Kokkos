cmake_minimum_required(VERSION 3.20)
project(kokkos_cuda_objs LANGUAGES CXX CUDA)

# Set standards: CUDA only supports up to C++17 as of CUDA 12.x
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_EXTENSIONS OFF)

# Do NOT find_package(Kokkos) here; the top-level provides Kokkos::kokkos.
find_package(OpenMP REQUIRED)

set(GPU_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/comm_gpu.cpp
)

# Treat .cpp as CUDA source so it’s compiled by NVCC
set_source_files_properties(${GPU_SOURCES} PROPERTIES LANGUAGE CUDA)

add_library(kokkos_cuda_objs STATIC ${GPU_SOURCES})

target_link_libraries(kokkos_cuda_objs
  PRIVATE
    Kokkos::kokkos
    OpenMP::OpenMP_CXX
)

target_include_directories(kokkos_cuda_objs
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(kokkos_cuda_objs PRIVATE KOKKOS_DEPENDENCE)

# Enable extended lambda and relaxed constexpr for Kokkos
target_compile_options(kokkos_cuda_objs PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda --expt-relaxed-constexpr -Xcompiler=-fopenmp>
)

# Make the library position independent (for linking into shared libs/executables)
set_target_properties(kokkos_cuda_objs PROPERTIES POSITION_INDEPENDENT_CODE ON)