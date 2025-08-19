#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <Kokkos_Core.hpp>

using HiClock = std::chrono::high_resolution_clock;

static double seconds_since(const HiClock::time_point& t0) {
    return std::chrono::duration<double>(HiClock::now() - t0).count();
}

static double checksum(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return s;
}

int main(int argc, char** argv) {
    // unsigned long long
    const std::size_t N = (argc > 1 ? std::stoull(argv[1]) : 50'000'000ULL);
    const int R = (argc > 2 ? std::stoi(argv[2]) : 5);
    const double a = 2.0;

    // --------------------------
    // KOKKOS: total (cold) vs compute-only (warm)
    // --------------------------
    double kokkos_total_sec = 0.0;
    double kokkos_compute_sec = 0.0;
    double sum_kokkos = 0.0;

    {
        const auto t_total0 = HiClock::now();             // start TOTAL timing (cold)
        Kokkos::initialize(argc, argv);
        {
            std::cout << "DefaultExecutionSpace = "
                      << Kokkos::DefaultExecutionSpace::name() << "\n";

            Kokkos::View<double*> x("x", N);
            Kokkos::View<double*> y("y", N);

            Kokkos::parallel_for("init", N, KOKKOS_LAMBDA(std::size_t i){
                x(i) = 1.0;
                y(i) = 0.0;
            });
            Kokkos::fence();

            Kokkos::Timer ktimer;                         // compute-only (warm) timing
            for (int r = 0; r < R; ++r) {
                Kokkos::parallel_for("saxpy", N, KOKKOS_LAMBDA(std::size_t i){
                    y(i) = a * x(i) + y(i);
                });
            }
            Kokkos::fence();
            kokkos_compute_sec = ktimer.seconds();

            auto y_h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), y);
            for (std::size_t i = 0; i < N; ++i) sum_kokkos += y_h(i);
        }
        Kokkos::finalize();
        kokkos_total_sec = seconds_since(t_total0);       // stop TOTAL timing (cold)
    }

    std::cout << "[Kokkos] N=" << N << " R=" << R
              << " total(cold)=" << kokkos_total_sec
              << " s, compute(warm)=" << kokkos_compute_sec
              << " s, checksum=" << sum_kokkos << "\n";

    // --------------------------
    // PURE OPENMP: total (cold) vs compute-only (warm)
    // --------------------------
    std::vector<double> x(N, 1.0), y(N, 0.0);

    const auto t_omp_total0 = HiClock::now();             // TOTAL timing (cold)
#pragma omp parallel for schedule(static)
    for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(N); ++i) {
        y[static_cast<std::size_t>(i)] = a * x[static_cast<std::size_t>(i)] + y[static_cast<std::size_t>(i)];
    }
    const double openmp_total_cold = seconds_since(t_omp_total0);

    const auto t_omp_warm0 = HiClock::now();              // compute-only (warm)
    for (int r = 1; r < R; ++r) {
#pragma omp parallel for schedule(static)
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(N); ++i) {
            y[static_cast<std::size_t>(i)] = a * x[static_cast<std::size_t>(i)] + y[static_cast<std::size_t>(i)];
        }
    }
    const double openmp_compute_warm = seconds_since(t_omp_warm0);

    const double sum_openmp = checksum(y);

    std::cout << "[OpenMP] N=" << N << " R=" << R
              << " total(cold)=" << openmp_total_cold
              << " s, compute(warm)=" << openmp_compute_warm
              << " s, checksum=" << sum_openmp << "\n";

    return 0;
}
