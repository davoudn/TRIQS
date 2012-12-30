from pytriqs.base.gf_local import *
from pytriqs.base.archive import *
from pytriqs.base.plot.mpl_interface import *

A = HDF_Archive("single_site_bethe.h5",'r')

for i in range(5):
  oplot(A['G-%s'%i]['up'].imag,'-o',Name='Iteration = %s'%i, x_window = (0,2))

