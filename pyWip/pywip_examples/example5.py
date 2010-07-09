#!/usr/bin/env python
# A plot showing how to use different panels

from pywip import *

viewport(0.2,0.9,0.3,0.8)  # Set the viewport
default(size=1.1) # Set the default character size

panel(1,nx=2,ny=2,gapx=3,start='bottom')
# Note that this curve will not appear in the legend
plot([3],[0],color='w',limits=(-0.1,6.4,-1.3,1.3),text=None)
axis(verticaly=True,drawtickx=False,drawticky=False)
axis(box=(),number=(),drawtickx=False,drawticky=False,gridx=True,gridy=True,
   style=':')

panel(2)
plot(1,2,'plotdata.dat',style='s',fillcolor='k',limits=(0.1,10,0.11,5),
   logx=True,logy=True,size=0.5,text='Data in panel 2')
errorbar(1,2,'plotdata.dat',yerr=4)
axis(gridx=True,gridy=True)

panel(3)
vector(1,2,3,4,'vector.dat',vent=0.35,size=0.25,limits=(-1.5,1.5,-1.5,1.5))
axis()

panel(4)
curve(text='Fake curve in panel 4',style='o')
axis()
legend(0.05,0.9,size=0.9)
# note the legend plots curves from all panels
savefig('plot-ex5.ps')
