#include <hpx/init.hpp>
#include "comm.hpp"
#include <iostream>

int hpx_main(int argc, char** argv) {
    comm_runtime_init(argc, argv);     
    {
        communicator c(8);
        c.print();
    }
    comm_runtime_finalize();           
    return hpx::local::finalize();
}

int main(int argc, char** argv) {
    return hpx::local::init(hpx_main, argc, argv);
}
