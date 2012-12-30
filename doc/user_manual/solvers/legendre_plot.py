from pytriqs.base.gf_local import *
from pytriqs.base.archive import *
from pytriqs.base.plot.mpl_interface import oplot

A = HDF_Archive("solution.h5")
oplot(A['Gl']['up'], '-o', x_window=(15,45) )
