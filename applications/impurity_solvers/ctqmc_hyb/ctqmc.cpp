
/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2011 by M. Ferrero, O. Parcollet
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

// include first because of a namespace clash.. to be fixed...
#include "ctqmc.hpp"
#include <triqs/arrays/h5/array_stack.hpp>
#include <triqs/python_tools/iterator_python_sequence.hpp>
#include <triqs/utility/callbacks.hpp>
 
// The moves to insert and remove C, Cdagger operators
#include "move_insert.hpp"
#include "move_insert_segment.hpp"

// The move to move operators
#include "move_shift.hpp"

// The Global moves
#include "move_global.hpp"

// The measures.
#include "measure_g.hpp"
#include "measure_f.hpp"
#include "measure_average.hpp"
#include "measure_legendre.hpp"
#include "measure_correlator.hpp"

using triqs::gf::gf_view;
using triqs::gf::block;
using triqs::gf::imtime;
using triqs::gf::legendre;

using namespace std;
using namespace boost;


template<typename T1> 
std::string make_string( T1 const & x1) { 
 std::stringstream fs; fs <<x1; return fs.str();
}
template<typename T1,typename T2> 
std::string make_string( T1 const & x1,T2 const & x2) { 
 std::stringstream fs; fs <<x1<<x2; return fs.str();
}


namespace triqs { namespace app { namespace impurity_solvers {

//-----------------------------------------------------

ctqmc_hyb::ctqmc_hyb(boost::python::object p, Hloc * hloc,
                     gf_view<block<imtime>> gt, gf_view<block<imtime>> ft,
                     gf_view<block<imtime>> dt, gf_view<block<imtime>> opcorr,
                     gf_view<block<legendre>> gl) :
 params(p),
 G_tau(gt), F_tau(ft), Delta_tau(dt), OpCorrToAverage(opcorr), G_legendre(gl),
 Config(params, hloc, Delta_tau),
 TimeAccumulation (params["Time_Accumulation"]),
 LegendreAccumulation (params["Legendre_Accumulation"]),
 N_Frequencies_Accu (params["N_Frequencies_Accumulated"]),
 Freq_Fit_Start (params["Fitting_Frequency_Start"]),
 QMC(params)
{

 const bool UseSegmentPicture (params["Use_Segment_Picture"]);

 // register the moves
 double p_ir = params["Proba_Insert_Remove"];
 double p_mv = params["Proba_Move"];

 typedef triqs::mc_tools::move_set<SignType> move_set_type;
 boost::shared_ptr<move_set_type> AllInserts(new move_set_type(QMC.RandomGenerator));
 boost::shared_ptr<move_set_type> AllRemoves(new move_set_type(QMC.RandomGenerator));
 for (int a =0; a<Config.Na;++a) { 

  if (UseSegmentPicture) {
   AllInserts->add( new Insert_Cdag_C_Delta_SegmentPicture ( a, Config, Histograms, QMC.RandomGenerator), make_string("Insert",a), 1.0);
   AllRemoves->add( new Remove_Cdag_C_Delta_SegmentPicture ( a, Config, QMC.RandomGenerator), make_string("Remove",a), 1.0);
  }  
  else {  
   AllInserts -> add( new Insert_Cdag_C_Delta ( a, Config, Histograms, QMC.RandomGenerator), make_string("Insert",a), 1.0);
   AllRemoves -> add( new Remove_Cdag_C_Delta ( a, Config, QMC.RandomGenerator), make_string("Remove",a), 1.0);
  }
 }

 QMC.add_move(AllInserts, "INSERT", p_ir);
 QMC.add_move(AllRemoves, "REMOVE", p_ir);
 QMC.add_move(new Move_C_Delta(Config, QMC.RandomGenerator), "Move C Delta", p_mv);

 // Register the Global moves
 python::list GM_List = python::extract<python::list>(params.dict()["Global_Moves_Mapping_List"]);
 for (triqs::python_tools::IteratorOnPythonListOf3Tuples<double,python::dict,string> g(GM_List); !g.atEnd(); ++g) {
  assert (python::len(g->x2)== Config.H.N_Operators());
  // transform a python dict : name_of_operator -> name_of_operator into a  
  vector<const Hloc::Operator*> mapping(Config.H.N_Operators(), (const Hloc::Operator*)NULL);
  for (triqs::python_tools::IteratorOnPythonDict<string,string> p(g->x2); !p.atEnd(); ++p) {
   mapping[Config.H[p->key].Number] =& Config.H[p->val];
   //cout<< "MAP" << Config.H[p->key].Number<< "  "<<mapping[Config.H[p->key].Number]->Number<<endl<<
   //      Config.H[p->key].name<< "  "<<mapping[Config.H[p->key].Number]->name<<endl;
  }
  QMC.add_move(new Global_Move(g->x3 , Config, QMC.RandomGenerator, mapping), "Global move", g->x1);
 }

 /*************

   Register the measures 

  ****************/

 for (int a =0; a<Config.Na;++a) { 
   if (LegendreAccumulation) {
     QMC.add_measure(new Measure_G_Legendre(Config, a, G_legendre[a]), make_string("G Legendre ",a));
     //if (bool(params["Keep_Full_MC_Series"])) 
      //QMC.add_measure(new Measure_G_Legendre_all(Config, a, G_legendre[a]), make_string("G Legendre (all) ",a));
   } else if (TimeAccumulation) {
     QMC.add_measure(new Measure_G_tau(Config, a, G_tau[a] ), make_string("G(tau) ",a));
   } else {
     assert(0);
   }
 }


 // register the measure of F
 if (bool(params["Use_F"]))
   for (int a =0; a<Config.Na;++a) 
     QMC.add_measure(new Measure_F_tau(Config, a, F_tau[a] ), make_string("F(tau) ",a));

 // register the measures of the average of some operators
 python::dict opAv_results = python::extract<python::dict>(params.dict()["Measured_Operators_Results"]);
 python::list opAv_List = python::extract<python::list>(params.dict()["Operators_To_Average_List"]);
 for (triqs::python_tools::IteratorOnPythonList<string> g(opAv_List); !g.atEnd(); ++g) {
  QMC.add_measure(new Measure_OpAv(*g, Config, opAv_results), *g);
 }

 // register the measures for the time correlators:
 python::list opCorr_List = python::extract<python::list>(params.dict()["OpCorr_To_Average_List"]);
 int a = 0;
 for (triqs::python_tools::IteratorOnPythonList<string> g(opCorr_List); !g.atEnd(); ++g, ++a) {
  string str1(*g);
  str1+= "OpCorr";
  QMC.add_measure(new Measure_OpCorr(str1, *g, Config, OpCorrToAverage[a], OpCorrToAverage[a].mesh().size()), str1);
 }

}


//********************************************************

void ctqmc_hyb::solve() {

  // communicate on the world = all nodes
  boost::mpi::communicator c;

  // run!! The empty configuration has sign = 1
  QMC.start(1.0, triqs::utility::clock_callback(params.value_or_default("MAX_TIME",-1)));
  QMC.collect_results(c);

}

}}}