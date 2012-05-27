/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2011 by O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#ifndef TRIQS_ARRAYS_MAPPED_FNT_H
#define TRIQS_ARRAYS_MAPPED_FNT_H
#include "./functional/map.hpp"
#include <boost/preprocessor/seq/for_each.hpp>

namespace triqs { namespace arrays {

 using std::abs;
 struct abs_wrap { 
   template<typename Sig> struct result;
   template<typename This, typename A> struct result<This(A)> { typedef A type;};
   template<typename This, typename A> struct result<This(std::complex<A>)> { typedef A type;};
   static const int arity =1;
   template<typename A> typename result<abs_wrap(A)>::type operator()( A const & a) const { return abs(a);}
 };

 template <typename A>
  typename boost::result_of<map_impl<abs_wrap>(A)>::type abs(A const & a) { return map(abs_wrap())(a); }

 using std::pow;
 struct pow_wrap { 
   template<typename Sig> struct result;
   template<typename This, typename A> struct result<This(A)> { typedef A type;};
   static const int arity =1;
   int n; pow_wrap(int n_): n(n_){}
   template<typename A> typename result<abs_wrap(A)>::type operator()( A const & a) const { return pow(a,n);}
 };

 template <typename A>
  typename boost::result_of<map_impl<pow_wrap>(A)>::type pow(A const & a, int n) { return map(pow_wrap(n))(a); }

#define MAP_IT(FNT) \
 using std::FNT;\
 struct FNT##_wrap {\
   template<typename Sig> struct result;\
   template<typename This, typename A> struct result<This(A)> { typedef A type;};\
   static const int arity =1;\
   template<typename A> typename result<FNT##_wrap(A)>::type operator()( A const & a) const { return FNT(a);}\
 };\
 template <typename A>\
  typename boost::result_of<map_impl<FNT##_wrap>(A)>::type FNT(A const & a) { return map(FNT##_wrap())(a); }

#define TRIQS_ARRAYS_MATH_FNT1 (cos)(sin)(tan)(cosh)(sinh)(tanh)(acos)(asin)(atan)(exp)(log)(sqrt)(floor)

#define AUX(r, data, elem) MAP_IT(elem)
 BOOST_PP_SEQ_FOR_EACH(AUX , nil , TRIQS_ARRAYS_MATH_FNT1);
#undef AUX  
#undef MAP_IT

}}//namespace triqs::arrays 
#endif

