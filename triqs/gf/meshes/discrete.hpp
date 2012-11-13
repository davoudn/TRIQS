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
#ifndef TRIQS_GF_MESH_DISCRETE_H
#define TRIQS_GF_MESH_DISCRETE_H
#include "./mesh_tools.hpp"
#include "../domains/discrete.hpp"

namespace triqs { namespace gf { 

  struct discrete_mesh {

   typedef discrete_domain domain_t;
   typedef size_t index_t; 

   discrete_mesh (domain_t && dom) : _dom(dom){}
   discrete_mesh () : _dom(){}
 
   domain_t const & domain() const { return _dom;}
   size_t size() const {return _dom.size();}

   /// Conversions point <-> index <-> discrete_index
   size_t  index_to_point (index_t ind) const {return ind;} 
   size_t  index_to_linear(index_t ind) const {return ind;}   

   /// The wrapper for the mesh point
   struct mesh_point_t : arith_ops_by_cast<mesh_point_t, size_t> {
    discrete_mesh const * m;  
    index_t index; 
    mesh_point_t( discrete_mesh const & mesh, index_t const & index_=0): m(&mesh), index(index_) {}
    void advance() { ++index;}
    operator size_t () const { return m->index_to_point(index);} 
   };

   /// Accessing a point of the mesh
   mesh_point_t operator[](index_t i) const { return mesh_point_t (*this,i);}

   /// Iterating on all the points...
   typedef  mesh_pt_generator<discrete_mesh> iterator;
   iterator begin() const { return iterator (this);}
   iterator end()   const { return iterator (this, true);}

   /// Mesh comparison
   bool operator == (discrete_mesh const & M) const { return (_dom == M._dom) ;} 

   /// Write into HDF5
   friend void h5_write (tqa::h5::group_or_file fg, std::string subgroup_name, discrete_mesh const & m) {
    tqa::h5::group_or_file gr =  fg.create_group(subgroup_name);
    h5_write(gr,"Domain",m.domain());
   }

   /// Read from HDF5
   friend void h5_read  (tqa::h5::group_or_file fg, std::string subgroup_name, discrete_mesh & m){
    tqa::h5::group_or_file gr = fg.open_group(subgroup_name);
    typename discrete_mesh::domain_t dom;
    size_t Nmax; 
    h5_read(gr,"Domain",m._dom);
    m = discrete_mesh(std::move(dom));
   }
 
   //  BOOST Serialization
   friend class boost::serialization::access;
   template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
     ar & boost::serialization::make_nvp("Domain",_dom);
    }

   private:
   domain_t _dom;
  };
}}
#endif

