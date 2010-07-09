#!/usr/bin/env python
# A plot to show the simplest method of making a halftone image with
# World coordinates
from pywip import *

default(size=1.1)
winadj('orion.fits')
halftone('orion.fits',minmax=(0,20),palette='-gray')
axis() # Note, since we plotted an image, axis defaults to wCS labeling
text('5:39','-4:50',r'T_{max}',size=1.5,align='center')
beam('5:32','-8:15','00:15','00:15',fillcolor='lg')
xlabel(r'\alpha (1950)')
ylabel(r'\delta (1950)',offset=1)
wedge(offset=0.5,thickness=3,verticaly=True)
savefig('plot-ex10.ps')
