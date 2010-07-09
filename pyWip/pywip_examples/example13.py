#!/usr/bin/env python
# The same as example 10, but now in color!
from pywip import *

default(size=1.1)
viewport(0.1,0.9,0.15,0.9)
rect(0,1,0,1,fillcolor='dg',fill='s')
viewport(0.25,0.75,0.2,0.9)
winadj(image='orion.fits')
halftone('orion.fits',header='rd',minmax=(0,20),palette='rainbow')
axis(color='y')
text('5:39','-4:50',r'T_{max}',size=1.5,align='center',color='r')
xlabel(r'\alpha (1950)',color='y')
ylabel(r'\delta (1950)',offset=1,color='y')
wedge('right',0.5,3,boxarg='bcmv',color='y')
savefig('plot-ex13.ps')
