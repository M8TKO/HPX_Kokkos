// 1) Let Kokkos see OpenMP normally
#include <Kokkos_Core.hpp>

// 2) Now hide _OPENMP only while parsing HPX headers
#ifdef _OPENMP
#  pragma push_macro("_OPENMP")
#  undef _OPENMP
#endif
#include <hpx/init.hpp>
#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#ifdef _OPENMP
#  pragma pop_macro("_OPENMP")
#endif

#include <iostream>
#include <vector>
#include <numeric>

// cmake -S . -B build-docker -DCMAKE_BUILD_TYPE=Release -DHPX_DIR=$HOME/hpx-install/lib/cmake/HPX
//  cmake --build build-docker -j 8
// OMP_NUM_THREADS=1 OMP_PROC_BIND=spread OMP_PLACES=threads ./build-docker/hpx_kokkos_demo --hpx:threads=8

int hpx_main(int argc, char** argv)
{
    // ---- HPX: simple parallel transform
    std::size_t N = 5'000'000;
    std::vector<double> v(N, 1.0);
    hpx::for_each(hpx::execution::par, v.begin(), v.end(),
                  [](double& x){ x = 2.0 * x + 1.0; });
    std::cout << "[HPX] sum=" << std::accumulate(v.begin(), v.end(), 0.0) << "\n";

    // ---- Kokkos: init + tiny kernel
    Kokkos::initialize(argc, argv);
    {
        std::cout << "Kokkos::DefaultExecutionSpace = "
                  << Kokkos::DefaultExecutionSpace::name() << "\n";
        Kokkos::View<double*> a("a", N);
        Kokkos::parallel_for("init", N, KOKKOS_LAMBDA(std::size_t i){ a(i) = 1.0; });
        Kokkos::fence();
    }
    Kokkos::finalize();
    return hpx::finalize();
}

int main(int argc, char** argv) { return hpx::init(argc, argv); }
