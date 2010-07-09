#!/usr/bin/env python
# A more complex layout with two viewports and a zoom box.
from pywip import *

viewport(0.2,0.55,0.2,1)
winadj('orion.fits')
default(size=1.15)
halftone('orion.fits',minmax=(0,20),palette='-gray')
axis(xinterval=(240,4),yinterval=(3600,4)) # Defaults to WCS labeling
xlabel(r'\alpha (1950)')
ylabel(r'\delta (1950)',offset=1)
text('5:39','-4:50',r'T_{max}',size=1.5,align='center')

blowup(100,201,160,321,corners=('ll','ur'),style=':')
viewport(0.7,0.95,0.2,1) #vy1,vy2
winadj('orion.fits[100:201,160:321]')
contour('orion.fits[100:201,160:321]',levels='10:40:10',limits=None)
# Note we want something slightly different than the default WCS labeling
# so we specify the parameters
axis(firsty=False,xinterval=(120,4),yinterval=(1800,3))
connect(corners=('ll','ur'),style='--')
savefig('plot-ex11.ps')
