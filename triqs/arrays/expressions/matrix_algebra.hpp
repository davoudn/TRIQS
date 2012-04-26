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

#ifndef TRIQS_ARRAYS_EXPRESSION_MATRIX_ALGEBRA_H
#define TRIQS_ARRAYS_EXPRESSION_MATRIX_ALGEBRA_H
#include "./algebra_common.hpp"
#include "../matrix.hpp"
#include "../linalg/matmul.hpp"
#include <boost/mpl/or.hpp>

namespace triqs { namespace arrays { namespace expressions { namespace matrix_algebra { 

 template<typename T> struct IsMatrix : Tag::check<Tag::matrix_algebra_expression_terminal,T> {}; 
 template<typename T> struct IsVector : Tag::check<Tag::vector_algebra_expression_terminal,T> {}; 
 template<typename T> struct IsMatrixOrVector : boost::mpl::or_<IsMatrix<T>, IsVector<T> > {};

 struct BasicVectorTypeGrammar :  proto::and_< proto::terminal<proto::_>, proto::if_<IsVector<proto::_value>()> > {}; 
 struct BasicMatrixTypeGrammar :  proto::and_< proto::terminal<proto::_>, proto::if_<IsMatrix<proto::_value>()> > {}; 

 typedef indexmaps::cuboid_domain<2> matrix_domain_type;

 template<typename T> struct wrap_scalar { 
  typedef T value_type; T val;
  typedef matrix_domain_type domain_type;
  wrap_scalar( T const &x ): val(x) {}
  domain_type domain() const { return domain_type();}
  value_type operator[](mini_vector<size_t,2> const & key) const { return (key[0]==key[1] ? val : T());}
  value_type operator()(size_t i, size_t j) const { return (i==j ? val : T());}
 };

template<typename T> struct wrap_vector { 
  typedef typename T::value_type value_type; 
  typedef matrix_domain_type domain_type;
  T const & x;
  wrap_vector( T const &x_): x(x_) {}
  domain_type domain() const { return domain_type(mini_vector<size_t,2>( x.domain().lengths()[0],1));}
 };

 template<class Tag, class V> struct _op;
 template<class V> struct _op<p_tag::plus,V> { static V invoke(V const & a, V const & b) { return a+b;}};
 template<class V> struct _op<p_tag::minus,V> { static V invoke(V const & a, V const & b) { return a-b;}};
 template<class V> struct _op<p_tag::multiplies,V> { static V invoke(V const & a, V const & b) { return a*b;}};
 template<class V> struct _op<p_tag::divides,V> { static V invoke(V const & a, V const & b) { return a/b;}};

 template<typename ProtoTag, typename T1, typename T2> struct matrix_add_sub_impl { 
  typedef typename T1::value_type V1; typedef typename T2::value_type V2; typedef BOOST_TYPEOF_TPL( V1() + V2()) value_type;
  typedef matrix_domain_type domain_type;
  T1 const & a; T2 const & b; matrix_add_sub_impl (T1 const & a_, T2 const & b_):a(a_),b(b_) {
   if (a.domain().lengths() != b.domain().lengths()) TRIQS_RUNTIME_ERROR << "Size mismatch in adding/substracting matrices"<< a.domain().lengths()<<b.domain().lengths();
  } 
  domain_type domain() const { return a.domain(); } 
  value_type operator[](mini_vector<size_t,2> const & key) const { return _op<ProtoTag,value_type>::invoke(a[key],b[key]);}
  value_type operator()(size_t i, size_t j) const { return _op<ProtoTag,value_type>::invoke(a(i,j),b(i,j));}
 }; 

 template<typename ProtoTag, typename T1, typename T2> struct matrix_sca_add_sub_impl { 
  typedef typename T1::value_type V1; typedef typename T2::value_type V2; typedef BOOST_TYPEOF_TPL( V1() + V2()) value_type;
  typedef matrix_domain_type domain_type;
  T1 const & a; T2 const & b; matrix_sca_add_sub_impl (T1 const & a_, T2 const & b_):a(a_),b(b_) {} 
  domain_type domain() const { return a.domain(); }
  value_type operator[](mini_vector<size_t,2> const & key) const { return  _op<ProtoTag,value_type>::invoke(a[key],b[key]);}
  value_type operator()(size_t i, size_t j) const { return _op<ProtoTag,value_type>::invoke(a(i,j),b(i,j));}
 };

 template<typename ProtoTag, typename T1, typename T2> struct matrix_sca_mul_div_impl { 
  typedef typename T1::value_type V1; typedef typename T2::value_type V2; typedef BOOST_TYPEOF_TPL( V1() + V2()) value_type;
  typedef matrix_domain_type domain_type;
  T1 const & a; T2 const & b; matrix_sca_mul_div_impl (T1 const & a_, T2 const & b_):a(a_),b(b_) {} 
  domain_type domain() const { return a.domain(); } 
  //value_type operator[](mini_vector<size_t,2> const & key) const { return  _op<ProtoTag,value_type>::invoke(a[key],b[mini_vector<size_t,2>()]);}
  //value_type operator()(size_t i, size_t j) const { return _op<ProtoTag,value_type>::invoke(a(i,j),b(0,0));}
  value_type operator[](mini_vector<size_t,2> const & key) const { return  _op<ProtoTag,value_type>::invoke(a[key],proto::value(b));}
  value_type operator()(size_t i, size_t j) const { return _op<ProtoTag,value_type>::invoke(a(i,j),proto::value(b));}
 }; 

#define SWITCH_GRAMMAR
#ifndef SWITCH_GRAMMAR

 struct MatrixGrammar : 
  proto::or_<
  proto::when< dC_ScalarGrammar,wrap_scalar<proto::_value>(proto::_value)>
  ,proto::when< BasicMatrixTypeGrammar,proto::_value>
  //,proto::when< BasicVectorTypeGrammar,wrap_vector<proto::_value>(proto::_value)>
  ,proto::or_<
  proto::when< proto::plus <dC_ScalarGrammar,MatrixGrammar>,matrix_sca_add_sub_impl<p_tag::plus,_right,_left> (_right,_left)>
  ,proto::when< proto::plus <MatrixGrammar,dC_ScalarGrammar>,matrix_sca_add_sub_impl<p_tag::plus,_left,_right> (_left,_right)>
  ,proto::when< proto::plus <MatrixGrammar,MatrixGrammar>,matrix_add_sub_impl<p_tag::plus,_left,_right> (_left,_right)>
  >
  ,proto::or_<
  proto::when< proto::minus <dC_ScalarGrammar,MatrixGrammar>,matrix_sca_add_sub_impl<p_tag::minus,_right,_left> (_right,_left)>
  ,proto::when< proto::minus <MatrixGrammar,dC_ScalarGrammar>,matrix_sca_add_sub_impl<p_tag::minus,_left,_right> (_left,_right)>
  ,proto::when< proto::minus <MatrixGrammar,MatrixGrammar>,matrix_add_sub_impl<p_tag::minus,_left,_right> (_left,_right)>
  >
  ,proto::or_<
  proto::when< proto::multiplies<dC_ScalarGrammar,MatrixGrammar>,matrix_sca_mul_div_impl< p_tag::multiplies,_right,_left> (_right,_left)>
  ,proto::when< proto::multiplies<MatrixGrammar,dC_ScalarGrammar>,matrix_sca_mul_div_impl< p_tag::multiplies,_left,_right> (_left,_right)>
  ,proto::when< proto::multiplies<MatrixGrammar,MatrixGrammar>,linalg::matmul_impl<_left,_right> (_left,_right)>
  >
  ,proto::when< proto::divides<MatrixGrammar,dC_ScalarGrammar>,matrix_sca_mul_div_impl< p_tag::divides,_left,_right> (_left,_right)>
  ,proto::when< proto::negate<MatrixGrammar>, MatrixGrammar(proto::_child0)>
  > {};

#else 
 struct MatrixGrammar;
 struct MatrixGrammarCases { template <typename TAG> struct case_: proto::not_<proto::_>{}; };

 template<> struct MatrixGrammarCases::case_<proto::tag::terminal> :  
  proto::or_<
  proto::when< dC_ScalarGrammar,wrap_scalar<proto::_value>(proto::_value)>
  ,proto::when< BasicMatrixTypeGrammar,proto::_value>
  > {};

 template<> struct MatrixGrammarCases::case_<proto::tag::plus> :  
  proto::or_<
  proto::when< proto::plus <dC_ScalarGrammar,MatrixGrammar>,matrix_sca_add_sub_impl<p_tag::plus,_right,_left> (_right,_left)>
  ,proto::when< proto::plus <MatrixGrammar,dC_ScalarGrammar>,matrix_sca_add_sub_impl<p_tag::plus,_left,_right> (_left,_right)>
  ,proto::when< proto::plus <MatrixGrammar,MatrixGrammar>,matrix_add_sub_impl<p_tag::plus,_left,_right> (_left,_right)>
  > {};

 template<> struct MatrixGrammarCases::case_<proto::tag::minus> :  
  proto::or_<
  proto::when< proto::minus <dC_ScalarGrammar,MatrixGrammar>,matrix_sca_add_sub_impl<p_tag::minus,_right,_left> (_right,_left)>
  ,proto::when< proto::minus <MatrixGrammar,dC_ScalarGrammar>,matrix_sca_add_sub_impl<p_tag::minus,_left,_right> (_left,_right)>
  ,proto::when< proto::minus <MatrixGrammar,MatrixGrammar>,matrix_add_sub_impl<p_tag::minus,_left,_right> (_left,_right)>
  > {};

 template<> struct MatrixGrammarCases::case_<proto::tag::multiplies> :  
  proto::or_<
  proto::when< proto::multiplies<dC_ScalarGrammar,MatrixGrammar>,matrix_sca_mul_div_impl< p_tag::multiplies,_right,_left> (_right,_left)>
  ,proto::when< proto::multiplies<MatrixGrammar,dC_ScalarGrammar>,matrix_sca_mul_div_impl< p_tag::multiplies,_left,_right> (_left,_right)>
  ,proto::when< proto::multiplies<MatrixGrammar,MatrixGrammar>,linalg::matmul_impl<_left,_right> (_left,_right)>
  > {};

 template<> struct MatrixGrammarCases::case_<proto::tag::divides> :  
  proto::or_<
  proto::when< proto::divides<MatrixGrammar,dC_ScalarGrammar>,matrix_sca_mul_div_impl< p_tag::divides,_left,_right> (_left,_right)>
  > {};

 template<> struct MatrixGrammarCases::case_<proto::tag::negate> :  
  proto::or_<
  proto::when< proto::negate<MatrixGrammar>, MatrixGrammar(proto::_child0)>
  > {};

 struct MatrixGrammar : proto::switch_<MatrixGrammarCases> {};

#endif


 /* ---------------------------------------------------------------------------------------------------
  * Define the main expression template ArrayExpr, the domain and Grammar (implemented below)
  * NB : it modifies the PROTO Domain to make *COPIES* of all objects.
  * We escape the copy of array by specializing the template below (end of file)
  * ---> to be rediscussed.
  * cf http://www.boost.org/doc/libs/1_49_0/doc/html/proto/users_guide.html#boost_proto.users_guide.front_end.customizing_expressions_in_your_domain.per_domain_as_child
  --------------------------------------------------------------------------------------------------- */
 template<typename Expr> struct MatrixExpr;
 struct MatrixDomain : proto::domain<proto::generator<MatrixExpr>, MatrixGrammar> {
  template< typename T > struct as_child : proto_base_domain::as_expr< T > {};
 };

 //For matrices and vectors, special treatment : the regular classes are replaced by the corresponding const view
 template< typename T, typename Opt> struct MatrixDomain::as_child< matrix<T,Opt> > : 
  MatrixDomain::proto_base_domain::template as_expr< const matrix_view<T,Opt> >{};

 template< typename T, typename Opt> struct MatrixDomain::as_child< const matrix<T,Opt> > : 
  MatrixDomain::proto_base_domain::template as_expr< const matrix_view<T,Opt> >{};

 template< typename T, typename Opt> struct MatrixDomain::as_child< vector<T,Opt> > : 
  MatrixDomain::proto_base_domain::template as_expr< const vector_view<T,Opt> >{};

 template< typename T, typename Opt> struct MatrixDomain::as_child< const vector<T,Opt> > : 
  MatrixDomain::proto_base_domain::template as_expr< const vector_view<T,Opt> >{};

 //   Expression
 template<typename Expr> struct MatrixExpr : Tag::expression, proto::extends<Expr, MatrixExpr<Expr>, MatrixDomain> { 
  typedef proto::extends<Expr, MatrixExpr<Expr>, MatrixDomain> base_type;
  typedef typename boost::remove_reference<typename boost::result_of<MatrixGrammar(Expr) >::type>::type  _T;
  typedef typename _T::value_type value_type;
  typedef typename _T::domain_type domain_type;
  typedef typename domain_type::index_value_type key_type;
  typedef size_t index_type;
  static const int rank = _T::domain_type::rank; static_assert((rank==2), "2 indices expected"); 

  MatrixExpr( Expr const & expr = Expr() ) : base_type( expr ) {} 

  domain_type domain() const { return MatrixGrammar()(*this).domain(); }
  size_t dim0() const { return this->domain().lengths()[0];} 
  size_t dim1() const { return this->domain().lengths()[1];} 
  mini_vector<size_t,2> shape() const { return this->domain().lengths();} 

  value_type operator[] (key_type const &key) const { 
   return MatrixGrammar()(*this)[key];
   //return proto::eval(*this, MatrixEvalCtx<key_type ,value_type> (key)); 
  }

  value_type operator()(index_type const & i1, index_type const & i2) const { return (*this)[mini_vector<size_t,2>(i1,i2)]; }

  friend std::ostream &operator <<(std::ostream &sout, MatrixExpr<Expr> const &expr){return proto::eval(expr,AlgebraPrintCtx(sout));}
 };
}}

BOOST_PROTO_DEFINE_OPERATORS(expressions::matrix_algebra::IsMatrix, expressions::matrix_algebra::MatrixDomain);
//BOOST_PROTO_DEFINE_OPERATORS(expressions::matrix_algebra::IsMatrixOrVector, expressions::matrix_algebra::MatrixDomain);

template<typename Expr > matrix_view <typename Expr::value_type> eval( Expr const & e) { return matrix<typename Expr::value_type>(e);}

}}//namespace triqs::arrays 

#endif

