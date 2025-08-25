#include "comm.hpp"
#include <Kokkos_Core.hpp>
#include <mutex>
#include <iostream>
#include <iomanip> 

// ---- Kokkos runtime control (initialize once, finalize once) ----
namespace {
    std::once_flag g_kokkos_once; //parameter for std::call_once
    bool g_finalize = false;
}

void comm_runtime_init(int& argc, char**& argv) {
    std::call_once(g_kokkos_once, [&]{
        Kokkos::initialize(argc, argv);
        std::cout << "Default Execution Space: " << Kokkos::DefaultExecutionSpace::name() << std::endl;
        g_finalize = true;
    });
}

void comm_runtime_finalize() {
    if (g_finalize && Kokkos::is_initialized()) {
        Kokkos::finalize();
        g_finalize = false;
    }
}

struct communicator::impl {
    Kokkos::DefaultExecutionSpace exec;
    Kokkos::View<double*> d;
    std::size_t n = 0;
};

// constructors / destructor
communicator::communicator() : p(new impl) {}

communicator::communicator(std::size_t n) : p(new impl) {
    resize(n);
}

communicator::~communicator() {
    delete p;
    p = nullptr;
}

// operations
void communicator::resize(std::size_t n) {
    p->d = Kokkos::View<double*>("comm_buf", n);
    p->n = n;

    auto v = p->d;
    Kokkos::parallel_for("init", static_cast<int>(n), KOKKOS_LAMBDA(int i){
        v(i) = 1.0;
    });
    Kokkos::fence();
}

void communicator::print() const {
     // What if we don't have any data?
    if (!p || p->n == 0){
    std::cout << "()\n"; 
    return;
    }
    auto h = Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), p->d);
    std::cout << "( " << h(0);
    for (std::size_t i = 1; i < p->n; ++i) 
         std::cout << std::setprecision(10) << ", " << h(i);
    std::cout << " )\n";
}

void communicator::calculation() const {
    auto v = p->d;
    auto n = p->n; 
    Kokkos::Timer timer;
    Kokkos::parallel_for("calculation",
      Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(p->exec, 0, static_cast<int>(n)),
      KOKKOS_LAMBDA (int i){
        for(int j = 0 ; j < 1e9 ; j++)
            v(i) += 0.001;
    });
    Kokkos::fence();
    std::cout << "Kokkos time: " << timer.seconds() << std::endl;
}

void communicator::fence() const { Kokkos::fence(); }

