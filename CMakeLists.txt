cmake_minimum_required(VERSION 3.20)
project(measuringTimeWithKokkos LANGUAGES CXX)

# ---- Basics
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Silence Boost policy warnings coming from HPX's CMake (optional)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

# ---- Dependencies
find_package(Kokkos REQUIRED)
find_package(OpenMP REQUIRED)               # we use Kokkos OpenMP backend
# If needed at configure time: -DHPX_DIR=$HOME/hpx-install/lib/cmake/HPX
set(HPX_DIR "$ENV{HOME}/hpx-install/lib/cmake/HPX" CACHE PATH "HPX cmake dir" FORCE)
find_package(HPX REQUIRED CONFIG)

# ---- Targets

# Your original Kokkos benchmark
# add_executable(measuringTimeWithKokkos src/measuringTimeWithKokkos.cpp)
# target_link_libraries(measuringTimeWithKokkos
#   PRIVATE
#     Kokkos::kokkos
#     HPX::hpx
#     OpenMP::OpenMP_CXX
# )

# HPX + Kokkos demo
# (Remember: in src/hpx_kokkos_demo.cpp we wrap HPX includes with push/pop of _OPENMP.)
add_executable(hpx_kokkos_demo src/hpx_kokkos_demo.cpp)
target_link_libraries(hpx_kokkos_demo
  PRIVATE
    Kokkos::kokkos
    HPX::hpx
    OpenMP::OpenMP_CXX
)

# Pick whichever HPX main/wrapper target exists on this install
if(TARGET HPX::wrap_main)
  target_link_libraries(hpx_kokkos_demo PRIVATE HPX::wrap_main)
elseif(TARGET HPX::hpx_init)
  target_link_libraries(hpx_kokkos_demo PRIVATE HPX::hpx_init)
elseif(TARGET HPX::hpx_main)
  target_link_libraries(hpx_kokkos_demo PRIVATE HPX::hpx_main)
else()
  message(FATAL_ERROR "No suitable HPX main/wrapper target found")
endif()

# (Optional) Group sources in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
  src/measuringTimeWithKokkos.cpp
  src/hpx_kokkos_demo.cpp
)
