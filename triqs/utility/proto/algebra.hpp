/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012 by O. Parcollet
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
#ifndef TRIQS_UTILITY_ALGEBRA_H
#define TRIQS_UTILITY_ALGEBRA_H 

#include <boost/type_traits/add_const.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/context.hpp>
#include <boost/proto/transform/arg.hpp>
#include <boost/proto/transform.hpp>
#include <boost/proto/domain.hpp>
//#include <boost/preprocessor/repetition/repeat.hpp>
//#include <boost/preprocessor/repetition/repeat_from_to.hpp>
//#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
//#include <boost/preprocessor/punctuation/comma.hpp>
//#include <boost/preprocessor/arithmetic/sub.hpp>
#include <complex>
#include "triqs/utility/typeid_name.hpp"
#include <assert.h>

namespace triqs { 

 template<typename T, typename Void =void> struct view_type_if_exists_else_type {typedef T type;}; 
 template<typename T> struct view_type_if_exists_else_type<T, typename T::has_view_type_tag> {typedef typename T::view_type type;}; 
 template<typename T> struct view_type_if_exists_else_type<const T, typename T::has_view_type_tag> {typedef const typename T::view_type type;}; 

}

namespace triqs { namespace utility { namespace proto { 

  namespace mpl = boost::mpl; namespace proto = boost::proto; namespace p_tag= proto::tag;

 template<typename T> struct is_in_ZRC : boost::is_arithmetic<T>  {};
 template<> struct is_in_ZRC<bool> : mpl::true_ {};
 template<typename T> struct is_in_ZRC<std::complex<T> > :  mpl::true_ {};

 // technical : evaluation of arithmetic operators
 template<typename TAG, typename A, typename B> struct _ops_;

 // the list of allowed operators in the grammar
#define BINARY_OP ((plus,+))((minus,-))((multiplies,*))((divides,/))

#define OP_NAME(elem) BOOST_PP_TUPLE_ELEM(2,0,elem)
#define OP_OP(elem)   BOOST_PP_TUPLE_ELEM(2,1,elem)

 // a trick to avoid putting T() in the typeof type deduction ! This code is NEVER used 
 template<class T> typename boost::unwrap_reference<T>::type pseudo_default_construct() { 
  typename boost::unwrap_reference<T>::type * x= NULL; assert(0); return *x; 
 }

#define AUX(r, data, elem) \
 template<typename A, typename B> struct _ops_<p_tag::OP_NAME(elem),A,B> {\
  typedef BOOST_TYPEOF_TPL( pseudo_default_construct<A>() OP_OP(elem) pseudo_default_construct<B>()) result_type;\
  static result_type invoke( A const &a , B const & b) { return a OP_OP(elem) b;} };
 BOOST_PP_SEQ_FOR_EACH(AUX, nil , BINARY_OP); 
#undef AUX

 template <typename T> std::ostream & formal_print(std::ostream & out, T const & x) { return out<<x;}

 template <typename T, typename A0> struct call_result_type { 
  typedef BOOST_TYPEOF_TPL (pseudo_default_construct<T>() (pseudo_default_construct<A0>())) type;
  };
 //template <typename T, typename A0> struct call_result_type<T,A0, typename T::template call_rtype<A0>::type> : T::template call_rtype<A0> {};

 namespace algebra { 

 template< typename OpsCompound, template<typename T> class is_element, template<typename T> class is_scalar = is_in_ZRC > 
  struct grammar_generator {

   struct LeafGrammar   : proto::and_< proto::terminal<proto::_>, proto::if_<is_element<proto::_value>()> > {}; 
   struct ScalarGrammar : proto::and_< proto::terminal<proto::_>, proto::if_<is_scalar<proto::_value>()> > {}; 
   struct Grammar : 
    proto::or_<
    proto::when< ScalarGrammar,                       typename OpsCompound::template scalar<proto::_value>(proto::_value) >
    ,proto::when< LeafGrammar,                        proto::_value >
    ,proto::when< proto::plus <Grammar,Grammar>,      typename OpsCompound::template binary_node<p_tag::plus,proto::_left,proto::_right > (proto::_left,proto::_right) >
    ,proto::when< proto::minus <Grammar,Grammar>,     typename OpsCompound::template binary_node<p_tag::minus,proto::_left,proto::_right > (proto::_left,proto::_right)  >
    ,proto::when< proto::multiplies<Grammar,Grammar>, typename OpsCompound::template binary_node<p_tag::multiplies,proto::_left,proto::_right > (proto::_left,proto::_right)>
    ,proto::when< proto::divides<Grammar,Grammar>,    typename OpsCompound::template binary_node<p_tag::divides,proto::_left,proto::_right > (proto::_left,proto::_right)>
    ,proto::when< proto::negate<Grammar >,            typename OpsCompound::template negate <proto::_left >(proto::_left) >
    > {};

   typedef Grammar type;
  };

  /* ---------------------------------------------------------------------------------------------------
   * The domain can enforce copies or not...
   * --------------------------------------------------------------------------------------------------- */
 
 template< typename Grammar, template<typename Expr> class The_Expr, bool CopyOrViewInsteadOfRef> struct domain;

 template< typename Grammar, template<typename Expr> class The_Expr> struct domain<Grammar,The_Expr,false> : proto::domain<proto::generator<The_Expr>, Grammar> { };
 //If true it modifies the PROTO Domain to make *COPIES* of ALL objects.
 //cf http://www.boost.org/doc/libs/1_49_0/doc/html/proto/users_guide.html#boost_proto.users_guide.front_end.customizing_expressions_in_your_domain.per_domain_as_child
 //Objects which have a view type are however NOT copied, the copy is replaced by a VIEW.
 template< typename Grammar, template<typename Expr> class The_Expr> struct domain<Grammar,The_Expr,true> : proto::domain<proto::generator<The_Expr>, Grammar> {
  //template< typename T > struct as_child : proto::domain<proto::generator<The_Expr>, Grammar>::proto_base_domain::template as_expr< T > {};
  template< typename T > struct as_child : 
   domain<Grammar,The_Expr,true>::proto_base_domain::template as_expr< typename boost::add_const<typename triqs::view_type_if_exists_else_type <T>::type >::type > {};
 };

 /* -------------------------------------------
  *  Structure of algebra for algebra valued functions
  * ------------------------------------------ */
 struct algebra_function_desc { 

  template<typename ProtoTag, typename L, typename R> struct binary_node  { 
   L const & l; R const & r; binary_node (L const & l_, R const & r_):l(l_),r(r_) {}
   template <typename T> struct call_rtype {
    typedef typename call_result_type<L,T>::type T1;
    typedef typename call_result_type<R,T>::type T2;
    typedef _ops_<ProtoTag, T1, T2 > ops_type;
    typedef typename ops_type::result_type type;
   };
   template<typename T> typename call_rtype<T>::type operator() (T const & arg) const {return call_rtype<T>::ops_type::invoke(l(arg),r(arg));}
  }; 

  template<typename S> struct scalar {
   S s; scalar( S const & x) : s(x) {}
   template <typename T> struct call_rtype { typedef S type; };
   template<typename T> typename call_rtype<T>::type operator() (T) const {return s;}
  };

  template<typename L> struct negate  { 
   L const & l; negate (L const & l_):l(l_) {} 
   template <typename T> struct call_rtype {
    typedef BOOST_TYPEOF_TPL ( (- pseudo_default_construct<L>() (pseudo_default_construct<T>()))) type;
   };
   template<typename T> typename call_rtype<T>::type operator() (T const & arg) const {return  (- l(arg));} 
  }; 
 };

 }

 /* -------------------------------------------
  *   Print context
  * ------------------------------------------ */

 struct AlgebraPrintCtx : proto::callable_context< AlgebraPrintCtx const > {
  typedef std::ostream &result_type;
  result_type out;
  AlgebraPrintCtx(std::ostream & out_):out(out_) {}
  template <typename T>
   typename boost::enable_if<is_in_ZRC<T>, result_type>::type 
   operator ()(proto::tag::terminal, const T & A) const { return out<<A; }
  template <typename T>
   typename boost::disable_if<is_in_ZRC<T>, result_type>::type 
   operator ()(proto::tag::terminal, const T & A) const { return out<< triqs::utility::typeid_name(A); }
  template<typename L, typename R> result_type operator ()(proto::tag::plus, L const &l, R const &r) const { return out << '(' << l << " + " << r << ')'; }
  template<typename L, typename R> result_type operator ()(proto::tag::minus, L const &l, R const &r) const { return out << '(' << l << " - " << r << ')'; }
  template<typename L, typename R> result_type operator ()(proto::tag::multiplies, L const &l, R const &r) const { return out << l << " * " << r; }
  template<typename L, typename R> result_type operator ()(proto::tag::divides, L const &l, R const &r) const { return out << l << " / " << r; }
 };


}}}

#undef OP_NAME
#undef OP_OP
#undef BINARY_OP

namespace triqs { 

 template<typename T> struct has_a_view_type: boost::mpl::false_{};
}

#endif
