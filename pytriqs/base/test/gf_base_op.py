
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

from pytriqs.base.archive import *
from pytriqs.base.gf_local import *
#import pytriqs.base.gf_local.gf_init as gf_init
import numpy, copy
from pytriqs.base.utility.my_utils import conjugate

h=HDFArchive('gf_base_op.output.h5','w')

ga = GfImFreq(indices = [1,2], beta = 50, n_matsubara = 100, name = "a1Block")
gb = GfImFreq(indices = [1,2], beta = 50, n_matsubara = 100, name = "b1Block")

G = BlockGf(name_list = ('a','b'), block_list = (ga,gb), make_copies = False)

#G <<= gf_init.A_Omega_Plus_B(1.0,2.0)
G <<= iOmega_n + 2.0
h['G1'] = G

# inverse:
G <<= inverse(G)

#  Density:
dens = G.total_density()
h['dens'] = dens

# FT:
f = lambda g,L : GfImTime(indices = g.indices, beta = g.beta, n_time_points =L )
gt = BlockGf(name_block_generator = [ (n,f(g,200) ) for n,g in G], make_copies=False, name='gt')
for (i,gtt) in gt : gtt.set_from_inverse_fourier(G[i])

h['gt'] = gt

# Matrix operations:
ga2 = GfImFreq(indices = [1,2,3], beta = 50, n_matsubara = 100, name = "a1Block")
mat = numpy.array([[1.0,0.0,1.0],[-1.0,1.0,0.0]])

ga2.from_L_G_R(mat.transpose(),ga,mat)

h['ga2'] = ga2


# conjugate:
Gc = conjugate(G)
h['Gc'] = Gc

# Some tail stuff:
tailtempl={}
for sig,g in G: 
    tailtempl[sig] = copy.deepcopy(g._tail)
h['tt'] = tailtempl


# tranpose
g = G['a']
gt = G['a'].transpose()
gt._data.array[0,1,3] = 100
assert g._data.array[1,0,3] == 100

del h
