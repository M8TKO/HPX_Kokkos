cmake_minimum_required(VERSION 3.20)
project(kokkos_cuda_objs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Kokkos REQUIRED)

add_library(kokkos_cuda_objs STATIC
  comm_gpu.cpp
)

target_link_libraries(kokkos_cuda_objs PRIVATE Kokkos::kokkos)

target_include_directories(kokkos_cuda_objs PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(kokkos_cuda_objs PRIVATE KOKKOS_DEPENDENCE)

set_target_properties(kokkos_cuda_objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
