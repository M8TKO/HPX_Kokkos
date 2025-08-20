#include <Kokkos_Core.hpp>
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
// OMP_NUM_THREADS=8 ./build/measuringTimeWithKokkos 10000000 5
// OMP_PROC_BIND=spread OMP_PLACES=threads ./rebuild.sh --run hpx_kokkos_demo --hpx:threads=8

#include <cstddef>
#include <iterator>
class view_iterator{
public:
    using difference_type = std::ptrdiff_t;
    using value_type = double;
    using reference = double&;
    using iterator_category = std::input_iterator_tag; 
private:
    Kokkos::View<double*> *data_ptr;
    difference_type i;
public:
    view_iterator() : i(0), data_ptr(nullptr) {}
    view_iterator(Kokkos::View<double*> *pdata, int j) : i(j), data_ptr(pdata) {}
    double& operator*() const {
        return (*data_ptr)[i];
    }
    view_iterator& operator++(){
        i++;
        return *this;
    }
    view_iterator operator++(int) { 
        view_iterator temp = *this;
        ++(*this); 
        return temp;
    }
    bool operator==(const view_iterator &it) const {
        return (data_ptr == it.data_ptr) && (i == it.i);
    }
    bool operator!=(const view_iterator &it) const {
        return !(it == *this);
    }
    ~view_iterator(){}
};
static_assert(std::input_iterator<view_iterator>);

int hpx_main(int argc, char** argv)
{   
    Kokkos::initialize(argc, argv);
    {
        std::size_t N = 1000;
        Kokkos::View<double*> a("a", N);

        Kokkos::parallel_for("init", N, KOKKOS_LAMBDA(std::size_t i){
            a(i) = 1.0;
        });

        view_iterator it1( &a, 0), it2( &a, N);
        
        hpx::for_each( it1, it2,
                    [](double& x){ x = x + 1.0; });
        double result = 0;
        Kokkos::parallel_reduce("reduce", N, KOKKOS_LAMBDA(std::size_t i, double& loc){
            loc += a(i);
        }, result);
        std::cout << "[HPX] sum=" << result << "\n";
    }
    
    Kokkos::finalize();
    return hpx::finalize();
}

int main(int argc, char** argv) { return hpx::init(argc, argv); }
