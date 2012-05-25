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
#ifndef TRIQS_ARRAYS_IMPL_TAGS_H
#define TRIQS_ARRAYS_IMPL_TAGS_H
#include <boost/type_traits/is_base_of.hpp>
	
namespace triqs { namespace arrays {

 //struct use_default {};

 namespace Tag {	
  template<typename TAG, typename T> struct check: boost::is_base_of<TAG,T> {};
  
  struct indexmap_storage_pair{};
  struct expression{};
  struct expression_terminal{};
  struct array_algebra_expression_terminal{};
  struct matrix_algebra_expression_terminal{};
  struct vector_algebra_expression_terminal{};
  struct scalar_expression_terminal{};

  struct has_special_assign{}; 
  template <char C> struct has_special_infix {}; 

  struct has_immutable_array_interface{};
  struct no_init {}; struct nan_inf_init {}; struct default_init {};   
  
  struct h5_array_proxy {};
 }

}}//namespace triqs::arrays
#endif

