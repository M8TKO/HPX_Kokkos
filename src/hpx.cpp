#include <hpx/init.hpp>
#include "comm.hpp"

#include <chrono>
#include <iostream>

int hpx_main(int argc, char** argv) {
    const int N = 5;
    comm_runtime_init(argc, argv);     
    {
        communicator c(N);
        c.print();
        c.calculation();
        c.print();
    }
    comm_runtime_finalize();
    return hpx::local::finalize();
}

int main(int argc, char** argv) {
    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();

    int ret = hpx::local::init(hpx_main, argc, argv);

    auto t1 = clock::now();
    auto elapsed = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "Total time (init → finalize): "
              << elapsed << " seconds\n";

    return ret;
}
