
################################################################################
#
# TRIQS: a Toolbox for Research in Interacting Quantum Systems
#
# Copyright (C) 2011 by M. Ferrero, O. Parcollet
#
# TRIQS is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# TRIQS. If not, see <http://www.gnu.org/licenses/>.
#
################################################################################

from numpy import array, arange
from math import pi
from cmath import sqrt, log
from pytriqs.Base.GF_Local import GFBloc_ImFreq, GFBloc_ReFreq
from pytriqs.Base.GF_Local.Descriptors import Function

beta = 100  # Inverse temperature
L = 101     # Number of Matsubara frequencies used in the Pade approximation
eta = 0.01  # Imaginary frequency shift

## Test Green's functions ##

# Two Lorentzians
def GLorentz(z):
    return 0.7/(z-2.6+0.3*1j) + 0.3/(z+3.4+0.1*1j)

# Semicircle
def GSC(z):
    return 2.0*(z + sqrt(1-z**2)*(log(1-z) - log(-1+z))/pi)

# A superposition of GLorentz(z) and GSC(z) with equal weights
def G(z):
    return 0.5*GLorentz(z) + 0.5*GSC(z)

# Matsubara GF
gm = GFBloc_ImFreq(Indices = [0], Beta = beta, Name = "gm")
gm <<= Function(G)
gm._tail.zero()
gm._tail[1] = array([[1.0]])

# Analytic continuation of gm
g_pade = GFBloc_ReFreq(Indices = [0], Beta = beta, MeshArray = arange(-6,6,0.01), Name = "g_pade")
g_pade.setFromPadeOf(gm, N_Matsubara_Frequencies = L, Freq_Shift = eta)

from pytriqs.Base.Archive import HDF_Archive
R = HDF_Archive('Pade.output.h5','w')
R['g_pade'] = g_pade
