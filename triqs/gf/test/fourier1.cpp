//#define TRIQS_ARRAYS_ENFORCE_BOUNDCHECK

#include <triqs/gf/imfreq.hpp> 
#include <triqs/gf/imtime.hpp> 
#include <triqs/gf/local/fourier_matsubara.hpp> 

namespace tql= triqs::clef;
namespace tqa= triqs::arrays;
using tqa::range;
using triqs::arrays::make_shape;
using triqs::gf::Fermion;
using triqs::gf::imfreq;
using triqs::gf::imtime;

#define TEST(X) std::cout << BOOST_PP_STRINGIZE((X)) << " ---> "<< (X) <<std::endl<<std::endl;

int main() {

 triqs::gf::freq_infty inf;

 double beta =1;
 auto G =  imfreq::make_gf (beta, Fermion, make_shape(2,2));
 auto Gc = imfreq::make_gf (beta, Fermion, make_shape(2,2));
 auto G3 = imfreq::make_gf (beta, Fermion, make_shape(2,2));
 auto Gt = imtime::make_gf (beta, Fermion, make_shape(2,2));

 //auto gt = inverse_fourier(G);
 //auto gw = fourier(gt);

 //gw() = lazy_fourier(gt);
 G() = lazy_fourier(Gt);
}


