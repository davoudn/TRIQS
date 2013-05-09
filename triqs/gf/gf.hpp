/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012-2013 by M. Ferrero, O. Parcollet
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
#ifndef TRIQS_GF_GFBASECLASS_H
#define TRIQS_GF_GFBASECLASS_H
#include <triqs/utility/first_include.hpp>
#include <triqs/utility/std_vector_expr_template.hpp>
#include <triqs/utility/factory.hpp>
#include <triqs/utility/tuple_tools.hpp>
#include <triqs/arrays/h5.hpp>
#include <vector>
#include "./tools.hpp"
#include "./data_proxies.hpp"

namespace triqs { namespace gf {
 using utility::factory;

 template<typename Descriptor> class gf;         // the value class
 template<typename Descriptor> class gf_view;    // the view class

 // gf_factories regroup all factories (constructors..) for all types of gf.
 // make_gf and make_gf_view forward any args to them
 template <typename Descriptor> struct gf_factories;
 template <typename Descriptor, typename ... U> gf<Descriptor> make_gf(U && ... x) { return gf_factories<Descriptor>::make_gf(std::forward<U>(x)...);}
 template <typename Descriptor, typename ... U> gf_view<Descriptor> make_gf_view(U && ... x) { return gf_factories<Descriptor>::make_gf_view(std::forward<U>(x)...);}

 // evaluator regroup functions to evaluate the function. Cf descriptors
 template<typename Descriptor> struct evaluator{ static constexpr int arity = 0;};

 // data_proxy contains function to manipulate the data array, but no data itself.
 // this is used to specialize this part of the code to array of dim 3 (matrix gf), dim 1 (scalar gf) and vector (e.g. block gf, ...)
 template<typename Descriptor, typename Enable = void> struct data_proxy;

 // This trait contains functions to read/write in hdf5 files. Can be specialized for some descriptor (Cf block)
 template <typename Descriptor> struct gf_h5_ops { 
  template<typename DataType, typename GF> static void write(h5::group g, std::string const & s, DataType const & data, GF const &) { h5_write(g,"data",data); }
  template<typename DataType, typename GF> static void read (h5::group g, std::string const & s, DataType & data, GF const &) { h5_read(g,"data",data);}
 };

 // The trait that "marks" the Green function
 TRIQS_DEFINE_CONCEPT_AND_ASSOCIATED_TRAIT(ImmutableGreenFunction);

 // ---------------------- implementation --------------------------------

 /// A common implementation class for gf and gf_view. They will only redefine contructor and = ...
 template<typename Descriptor,bool IsView> class gf_impl : TRIQS_MODEL_CONCEPT(ImmutableGreenFunction), Descriptor::tag {
  public :

   // Pattern : ValueView
   typedef gf_view<Descriptor> view_type;
   typedef gf<Descriptor>      non_view_type;

   typedef Descriptor                         descriptor_t;
   typedef typename Descriptor::mesh_t        mesh_t;
   typedef typename mesh_t::domain_t          domain_t;
   typedef typename mesh_t::mesh_point_t      mesh_point_t;
   typedef typename mesh_t::index_t           mesh_index_t;
   typedef typename Descriptor::symmetry_t    symmetry_t;
   typedef evaluator<Descriptor>              evaluator_t;

   typedef data_proxy<Descriptor>                                                 data_proxy_t;
   typedef typename data_proxy_t::storage_t                                       data_non_view_t;
   typedef typename data_proxy_t::storage_view_t                                  data_view_t;
   typedef typename std::conditional<IsView, data_view_t, data_non_view_t>::type  data_t;

   typedef typename Descriptor::singularity_t                                                  singularity_non_view_t;
   typedef typename view_type_if_exists_else_type<singularity_non_view_t>::type                singularity_view_t;
   typedef typename std::conditional<IsView, singularity_view_t, singularity_non_view_t>::type singularity_t;

   mesh_t const &        mesh()          const { return _mesh;}
   domain_t const &      domain()        const { return _mesh.domain();}
   data_t &              data()                { return _data;}
   data_t const &        data()          const { return _data;}
   singularity_t &       singularity()         { return _singularity;}
   singularity_t const & singularity()   const { return _singularity;}
   symmetry_t const &    symmetry()      const { return _symmetry;}
   evaluator_t const &   get_evaluator() const { return _evaluator;}

  protected:
   mesh_t        _mesh;
   data_t        _data;
   singularity_t _singularity;
   symmetry_t    _symmetry;
   evaluator_t   _evaluator;
   data_proxy_t  _data_proxy;

   // --------------------------------Constructors -----------------------------------------------
   // all protected but one, this is an implementation class, see gf/gf_view later for public one
   gf_impl() {} // all arrays of zero size (empty)

  public : //everyone can make a copy (for clef lib in particular, this one needs to be public)
   
   gf_impl(gf_impl const & x) : _mesh(x.mesh()), _data(factory<data_t>(x.data())),
   _singularity(factory<singularity_t>(x.singularity())), _symmetry(x.symmetry()), _evaluator(x._evaluator){}

   gf_impl(gf_impl &&) = default;

  protected:
  
   gf_impl(gf_impl<Descriptor,!IsView> const & x): _mesh(x.mesh()), _data(factory<data_t>(x.data())),
   _singularity(factory<singularity_t>(x.singularity())), _symmetry(x.symmetry()), _evaluator(x.get_evaluator()){}

   template<typename M, typename D, typename S, typename SY, typename EV> 
   gf_impl(M && m, D && dat, S && sing, SY && sy, EV && ev) : 
    _mesh(std::forward<M>(m)), _data(std::forward<D>(dat)), _singularity(std::forward<S>(sing)),_symmetry(std::forward<SY>(sy)), _evaluator(std::forward<EV>(ev)){}

   void operator = (gf_impl const & rhs) = delete; // done in derived class.

   void swap_impl (gf_impl & b) noexcept {
    using std::swap;
    swap(this->_mesh, b._mesh); swap(this->_data, b._data); swap (this->_singularity,b._singularity); swap(this->_symmetry,b._symmetry); swap(this->_evaluator,b._evaluator);
   }

  public:
   // ------------- All the call operators -----------------------------

   // First, a simple () returns a view, like for an array...
   view_type operator()() const { return *this;}

   /// Calls are (perfectly) forwarded to the Descriptor::operator(), except mesh_point_t and when
   /// there is at least one lazy argument ...
   template<typename Arg0, typename... Args >    // match any argument list, picking out the first type : () is not permitted
    typename std::add_const<
    typename boost::lazy_disable_if<  // disable the template if one the following conditions it true
    boost::mpl::or_< // starting condition [OR]
    std::is_base_of< tag::mesh_point, typename std::remove_reference<Arg0>::type>,  // Arg0 is (a & or a &&) to a mesh_point_t
    clef::is_any_lazy<Arg0, Args...>                          // One of Args is a lazy expression
     , boost::mpl::bool_<(sizeof...(Args)!= evaluator_t::arity -1 ) && (evaluator_t::arity !=-1)> // if -1 : no check
     >,                                                       // end of OR
    std::result_of<evaluator_t(gf_impl*,Arg0, Args...)> // what is the result type of call
     >::type     // end of lazy_disable_if
     >::type // end of add_Const
     operator() (Arg0&& arg0, Args&&... args) const { return _evaluator(this,std::forward<Arg0>( arg0), std::forward<Args>(args)...); }

   // Interaction with the CLEF library : calling the gf with any clef expression as argument build a new clef expression
   template<typename Arg0, typename ...Args>
    typename clef::result_of::make_expr_call<view_type,Arg0, Args...>::type
    //typename clef::result_of::make_expr_call<gf_impl,Arg0, Args...>::type
    operator()(Arg0 arg0, Args... args) const {
     return clef::make_expr_call(view_type(*this),arg0, args...);
    }

   typedef typename std::result_of<data_proxy_t(data_t       &,size_t)>::type r_type;
   typedef typename std::result_of<data_proxy_t(data_t const &,size_t)>::type cr_type;

   r_type  operator() (mesh_point_t const & x)       { return _data_proxy(_data, x.linear_index());}
   cr_type operator() (mesh_point_t const & x) const { return _data_proxy(_data, x.linear_index());}

   // on mesh component for composite meshes
   // enable iif the first arg is a mesh_point_t for the first component of the mesh_t
   template<typename Arg0, typename ... Args, bool MeshIsComposite = std::is_base_of<tag::composite, mesh_t>::value >
    typename std::enable_if< MeshIsComposite && std::is_base_of< tag::mesh_point, Arg0>::value,  r_type>::type
    operator() (Arg0 const & arg0, Args const & ... args)
    { return _data_proxy(_data, _mesh.mesh_pt_components_to_linear(arg0, args...));}

   template<typename Arg0, typename ... Args, bool MeshIsComposite = std::is_base_of<tag::composite, mesh_t>::value >
    typename std::enable_if< MeshIsComposite && std::is_base_of< tag::mesh_point, Arg0>::value,  cr_type>::type
    operator() (Arg0 const & arg0, Args const & ... args) const
    { return _data_proxy(_data, _mesh.mesh_pt_components_to_linear(arg0, args...));}

   /// A direct access to the grid point
   template<typename... Args>
    r_type on_mesh (Args&&... args) { return _data_proxy(_data,_mesh.index_to_linear(mesh_index_t(std::forward<Args>(args)...)));}

   /// A direct access to the grid point (const version)
   template<typename... Args>
    cr_type on_mesh (Args&&... args) const { return _data_proxy(_data,_mesh.index_to_linear(mesh_index_t(std::forward<Args>(args)...)));}

  private:
   struct _on_mesh_wrapper_const {
    gf_impl const & f; _on_mesh_wrapper_const (gf_impl const & _f) : f(_f) {}
    template <typename... Args> cr_type operator ()(Args && ... args) const { return f.on_mesh(std::forward<Args>(args)...);}
   };
   struct _on_mesh_wrapper {
    gf_impl & f; _on_mesh_wrapper (gf_impl & _f) : f(_f) {}
    template <typename... Args> r_type  operator ()(Args && ... args) const { return f.on_mesh(std::forward<Args>(args)...);}
   };
   _on_mesh_wrapper_const friend on_mesh(gf_impl const & f) { return f;}
   _on_mesh_wrapper friend on_mesh(gf_impl & f) { return f;}

  public:
   r_type  operator[] (mesh_index_t const & arg)       { return _data_proxy(_data,_mesh.index_to_linear(arg));}
   cr_type operator[] (mesh_index_t const & arg) const { return _data_proxy(_data,_mesh.index_to_linear(arg));}

   // Interaction with the CLEF library : calling the gf with any clef expression as argument build a new clef expression
   template<typename Arg>
    typename boost::lazy_enable_if<    // enable the template if
    clef::is_any_lazy<Arg>,  // One of Args is a lazy expression
    clef::result_of::make_expr_call<view_type,Arg>
     >::type     // end of lazy_enable_if
     operator[](Arg && arg) const { return clef::make_expr_call(view_type(*this),std::forward<Arg>(arg));}

   //----------------------------- HDF5 -----------------------------

   friend std::string get_triqs_hdf5_data_scheme(gf_impl const & g) { return descriptor_t::h5_name();}

   /// Write into HDF5
   friend void h5_write (h5::group fg, std::string subgroup_name, gf_impl const & g) {
    auto gr =  fg.create_group(subgroup_name);
    gr.write_triqs_hdf5_data_scheme(g); 
    gf_h5_ops<Descriptor>::write(gr, "data", g._data, g);//can be specialized for some descriptors (E.g. blocks)
    h5_write(gr,"singularity",g._singularity);
    h5_write(gr,"mesh",g._mesh);
    h5_write(gr,"symmetry",g._symmetry);
   }

   /// Read from HDF5
   friend void h5_read (h5::group fg, std::string subgroup_name, gf_impl & g) {
    auto gr = fg.open_group(subgroup_name);
    // Check the attribute or throw
    auto tag_file = gr.read_triqs_hdf5_data_scheme();
    auto tag_expected= get_triqs_hdf5_data_scheme(g);
    if (tag_file != tag_expected) 
     TRIQS_RUNTIME_ERROR<< "h5_read : mismatch of the tag TRIQS_HDF5_data_scheme tag in the h5 group : found "<<tag_file << " while I expected "<< tag_expected; 
    gf_h5_ops<Descriptor>::read(gr, "data", g._data, g);//can be specialized for some descriptors (E.g. blocks)
    h5_read(gr,"singularity",g._singularity);
    h5_read(gr,"mesh",g._mesh);
    h5_read(gr,"symmetry",g._symmetry);
   }

   //-----------------------------  BOOST Serialization -----------------------------
   friend class boost::serialization::access;
   template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
     ar & boost::serialization::make_nvp("data",_data);
     ar & boost::serialization::make_nvp("singularity",_singularity);
     ar & boost::serialization::make_nvp("mesh",_mesh);
     ar & boost::serialization::make_nvp("symmetry",_symmetry);
    }

   /// print
   friend std::ostream & operator << (std::ostream & out, gf_impl const & x) { return out<<(IsView ? "gf_view": "gf");}
   friend std::ostream & triqs_nvl_formal_print(std::ostream & out, gf_impl const & x) { return out<<(IsView ? "gf_view": "gf");}
 };

 // ---------------------------------------------------------------------------------
 ///The regular class of GF
 template<typename Descriptor> class gf : public gf_impl<Descriptor,false> {
  typedef gf_impl<Descriptor,false> B;
  public :

  gf():B() {}
  gf(gf const & g): B(g){}
  gf(gf && g) noexcept : B(std::move(g)){}
  gf(gf_view<Descriptor> const & g): B(g){}
  template<typename GfType> gf(GfType const & x): B() { *this = x;}

  template<typename DataViewType> // anything from which the factory can make the data ...
   gf(typename B::mesh_t const & m,
     DataViewType && dat,
     typename B::singularity_view_t const & si,
     typename B::symmetry_t const & s ,
     typename B::evaluator_t const & eval = typename B::evaluator_t ()
     ) :
    B(m,factory<typename B::data_t>(std::forward<DataViewType>(dat)),si,s,eval) {}

  friend void swap (gf & a, gf & b) noexcept { a.swap_impl (b);}

  gf & operator = (gf const & rhs) { *this = gf(rhs); return *this;} // use move =
  gf & operator = (gf & rhs) { *this = gf(rhs); return *this;} // use move =
  gf & operator = (gf && rhs) noexcept { swap(*this, rhs); return *this;}

  template<typename RHS> void operator = (RHS && rhs) {
   this->_mesh = rhs.mesh();
   B::data_proxy_t::assign_with_resize(this->data(), std::forward<RHS>(rhs).data()); // looks strange for &&
   this->_singularity = rhs.singularity();
   // to be implemented : there is none in the gf_expr in particular....
   //this->_symmetry = rhs.symmetry(); 
  }
 };

 // ---------------------------------------------------------------------------------
 ///The View class of GF
 template<typename Descriptor> class gf_view : public gf_impl<Descriptor,true> {
  typedef gf_impl<Descriptor,true> B;
  public :
  gf_view(gf_view const & g): B(g){}
  gf_view(gf_view && g) noexcept : B(std::move(g)){}

  template<bool V> gf_view(gf_impl<Descriptor,V> const & g): B(g){}

  template<typename D, typename T>
   gf_view (typename B::mesh_t const & m,
     D const & dat,T const & t,typename B::symmetry_t const & s,
     typename B::evaluator_t const &e = typename B::evaluator_t ()  ) :
    B(m,factory<typename B::data_t>(dat),t,s,e) {}

  friend void swap (gf_view & a, gf_view & b) noexcept { a.swap_impl (b);}

  void rebind (gf_view const &X) noexcept {
   this->_mesh = X._mesh; this->_symmetry = X._symmetry; 
   this->_data_proxy.rebind(this->_data,X); 
   this->_singularity.rebind(X._singularity);
  }

  gf_view & operator = (gf_view const & rhs)  { triqs_gf_view_assign_delegation(*this,rhs); return *this;}

  template<typename RHS> gf_view & operator = (RHS const & rhs) { triqs_gf_view_assign_delegation(*this,rhs); return *this;}

  // Interaction with the CLEF library : auto assignment of the gf (gf(om_) << expression fills the functions by evaluation of expression)
  template<typename RHS> friend void triqs_clef_auto_assign (gf_view & g, RHS rhs) {
   // access to the data . Beware, we view it as a *matrix* NOT an array... (crucial for assignment to scalars !)
   g.triqs_clef_auto_assign_impl(rhs, typename std::is_base_of<tag::composite,typename B::mesh_t>::type());
   assign_from_expression(g.singularity(),rhs);
   // if f is an expression, replace the placeholder with a simple tail. If f is a function callable on freq_infty,
   // it uses the fact that tail_non_view_t can be casted into freq_infty
  }

  private:
  template<typename RHS> void triqs_clef_auto_assign_impl (RHS const & rhs, std::integral_constant<bool,false>) {
   for (auto const & w: this->mesh()) (*this)(w) = rhs(w);
  }
  template<typename RHS> void triqs_clef_auto_assign_impl (RHS const & rhs, std::integral_constant<bool,true>) {
   for (auto const & w: this->mesh()) (*this)(w) = triqs::tuple::apply(rhs,w.components_tuple());
   //for (auto w: this->mesh()) triqs::tuple::apply(*this,w.components_tuple()) = triqs::tuple::apply(rhs,w.components_tuple());
  }

 }; // class gf_view

 // delegate = so that I can overload it for specific RHS...
 template<typename Descriptor, typename RHS>
  DISABLE_IF(arrays::is_scalar<RHS>) triqs_gf_view_assign_delegation( gf_view<Descriptor> & g, RHS const & rhs) {
   if (!(g.mesh()  == rhs.mesh()))  TRIQS_RUNTIME_ERROR<<"Gf Assignment in View : incompatible mesh";
   gf_view<Descriptor>::data_proxy_t::assign_no_resize(g.data(),rhs.data()); 
   g.singularity() = rhs.singularity();
  }

 template<typename Descriptor, typename T>
  ENABLE_IF(arrays::is_scalar<T>) triqs_gf_view_assign_delegation( gf_view<Descriptor> & g, T const & x) {
   gf_view<Descriptor>::data_proxy_t::assign_to_scalar(g.data(), x); 
   g.singularity() = x;
  }

 // tool for lazy transformation
 template<typename Tag, typename D> struct gf_keeper{ gf_view<D> g; gf_keeper (gf_view<D> const & g_) : g(g_) {} };

 // ---------------------------------- slicing ------------------------------------

 template<typename Descriptor, bool V, typename... Args>
  gf_view<Descriptor> slice_target (gf_impl<Descriptor,V> const & g, Args... args) {
   return gf_view<Descriptor>(g.mesh(), g.data()(tqa::range(), args... ), slice_target (g.singularity(),args...), g.symmetry());
  }

 template<typename Descriptor, bool V, typename... Args>
  gf_view<Descriptor> slice_mesh (gf_impl<Descriptor,V> const & g, Args... args) {
   return gf_view<Descriptor>(g.mesh().slice(args...), g.data()(g.mesh().slice_get_range(args...),arrays::ellipsis()), g.singularity(), g.symmetry());
  }

}}

#include "./gf_expr.hpp"
#endif
