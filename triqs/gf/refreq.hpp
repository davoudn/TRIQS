/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012 by M. Ferrero, O. Parcollet
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
#ifndef TRIQS_GF_FREQ_H
#define TRIQS_GF_FREQ_H
#include "./tools.hpp"
#include "./gf.hpp"
#include "./local/tail.hpp"
#include "./gf_proto.hpp"
#include "./meshes/linear.hpp"

// Shall we use the same type as for retime : same code, almost ??

namespace triqs { namespace gf {

 struct refreq {

  /// A tag to recognize the function 
  struct tag {};

  /// The domain
  struct domain_t {
   typedef double point_t; 
   bool operator == (domain_t const & D) const { return true; }
   friend void h5_write (tqa::h5::group_or_file fg, std::string subgroup_name, domain_t const & d) {}
   friend void h5_read  (tqa::h5::group_or_file fg, std::string subgroup_name, domain_t & d){ }
   friend class boost::serialization::access;
   template<class Archive> void serialize(Archive & ar, const unsigned int version) {}
  };

  /// The Mesh
  typedef linear_mesh<domain_t> mesh_t;

  /// The target
  typedef arrays::matrix<std::complex<double> >     target_t;
  //  typedef arrays::matrix<std::complex<double>, arrays::Option::Fortran >     target_t;
  typedef typename target_t::view_type                                       target_view_t;

  /// The tail
  typedef local::tail   singularity_t;

  /// Symmetry
  typedef nothing symmetry_t;

  /// Indices
  typedef indices_2_t indices_t;

  /// Arity (number of argument in calling the function)
  static const int arity =1;

  /// All the possible calls of the gf
  struct evaluator { 
   template<typename D, typename T>
    target_view_t operator() (mesh_t const & mesh, D const & data, T const & t, double w0)  const {
     size_t index; double w; bool in;
     std::tie(in, index, w) = windowing(mesh,w0);
     if (!in) TRIQS_RUNTIME_ERROR <<" Evaluation out of bounds";
     //return data(arrays::ellipsis(),mesh.index_to_linear(index)); 
     target_t res = w*data(arrays::ellipsis(),mesh.index_to_linear(index)) + (1-w)*data(arrays::ellipsis(),mesh.index_to_linear(index+1));
     return res;
    } 

   template<typename D, typename T>
    local::tail_view operator()(mesh_t const & mesh, D const & data, T const & t, freq_infty const &) const {return t;} 
  };

  struct bracket_evaluator {};

  /// How to fill a gf from an expression (RHS)
  template<typename D, typename T, typename RHS> 
   static void assign_from_expression (mesh_t const & mesh, D & data, T & t, RHS rhs) { 
    for (size_t u=0; u<mesh.size(); ++u)  { target_view_t( data(tqa::range(),tqa::range(),u)) = rhs(mesh[u]); }
    t = rhs( local::tail::omega(t.shape(),t.size()));
   }

  static std::string h5_name() { return "real_freq";}

  // -------------------------------   Factories  --------------------------------------------------

  typedef gf<refreq> gf_t;

  static gf_t make_gf(double wmin, double wmax, size_t n_freq, tqa::mini_vector<size_t,2> shape) { 
   refreq::mesh_t m(refreq::domain_t(),wmin, wmax,n_freq);
   gf_t::data_non_view_t A(shape.append(m.size())); A() =0;
   return gf_t (m, std::move(A), local::tail(shape), nothing(), indices_t() ) ;
  }

 };

 // -------------------------------   Expression template --------------------------------------------------

 // A trait to identify objects that have the concept ImmutableGfFreq
 template<typename G> struct ImmutableGfFreq : boost::is_base_of<typename refreq::tag,G> {};  

 // This defines the expression template with boost::proto (cf gf_proto.hpp).
 // TRIQS_GF_DEFINE_OPERATORS(refreq,local::is_scalar_or_element,ImmutableGfFreq);

}}
#endif

