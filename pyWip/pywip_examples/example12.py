#!/usr/bin/env python
# A plot showing how to overlay two different images on top of each other.
from pywip import *

viewport(0.25,0.75,0.2,0.9)
default(size=1.1)
winadj(image='orion.fits')
halftone('orion.fits',header='rd',minmax=(0,20),palette='-gray')
contour('iras60.fits',header='rd',levels='border')       # draw the border
contour('iras60.fits',header='rd',levels=[40,60,80,100]) # contours
axis()
text('5:39','-4:50',r'^{12}CO T_{max}',size=1.5,align='center')
text('5:39','-6:40',r'60 \mum',size=1.5,align='left')
xlabel(r'\alpha (1950)')
ylabel(r'\delta (1950)',offset=1)
savefig('plot-ex12.ps')
