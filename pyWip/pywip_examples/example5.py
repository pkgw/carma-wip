#!/usr/bin/env python

from pywip import *

def mybox(xbox,ybox,lstyle,xtick=None,ytick=None):
   axis(xbox,ybox,xtick=xtick,ytick=ytick)
   if 'l' in xbox:
      axis('gl','gl',style=lstyle,xtick=xtick,ytick=ytick)
   else:
      axis('g','g',style=lstyle,xtick=xtick,ytick=ytick)

viewport(0.2,0.9,0.3,0.8)  # Set the viewport
default(size=1.1)          # Set the default character size

panel(2,2,1)
plot([10],[10],limits=(-0.1,6.4,-1.3,1.3)) # dummy plot to set limits
mybox('bcn','bcnv',':')  # Draw lower left box

panel(2,2,2)
plot([10],[10],limits=(-0.1,6.4,-1.3,1.3)) # dummy plot to set limits
mybox('bcn','bcnv','--',xtick=(1,5),ytick=(0.3,3)) # Draw lower right box

panel(2,2,3)
plot(1,2,'data-ex5.dat',style='fs',limits=(0.1,10,0.1,2),logx=True,logy=True)
mybox('bcnl','bncvl','.-') # Draw upper right box

panel(2,2,4)
plot([10],[10],limits=1,logx=True,logy=True) # dummy plot to set limits
mybox('bcnl','bcnvl',':')  # Draw upper left box
savefig('plot-ex5.ps')
