cmake_minimum_required(VERSION 3.20)
project(kokkos_cuda_objs LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
target_compile_options(kokkos_cuda_objs PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda --expt-relaxed-constexpr -Xcompiler=-fopenmp>
)
# target_compile_options(kokkos_cuda_objs PRIVATE
#   $<$<COMPILE_LANGUAGE:CUDA>:
#     -arch=sm_86
#     -code=sm_86
#     --expt-extended-lambda
#     --expt-relaxed-constexpr
#     -Xcompiler=-fopenmp
#   >
# )
# Make life easier when linking into the HPX exe
set_target_properties(kokkos_cuda_objs PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Ensure NVCC’s host compilation sees the OpenMP flag
# (GCC/Clang host: -fopenmp)
target_compile_options(kokkos_cuda_objs PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-fopenmp>
)
