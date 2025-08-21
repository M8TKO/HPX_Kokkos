cmake_minimum_required(VERSION 3.20)
project(HPX_KokkosWithCuda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

# ---- Dependencies (host side) ----
find_package(OpenMP REQUIRED)

# HPX
set(HPX_DIR "$ENV{HOME}/hpx-install/lib/cmake/HPX" CACHE PATH "HPX cmake dir")
find_package(HPX REQUIRED CONFIG)

# Kokkos (headers needed on HPX side because comm.hpp includes <Kokkos_Core.hpp>)
find_package(Kokkos REQUIRED)

# ---- Subproject: CUDA/Kokkos implementation (compiled with nvcc_wrapper inside gpu/) ----
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
