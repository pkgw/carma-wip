#!/usr/bin/env python
# An example to illustrate basic commands
from pylab import *
from matplotlib.patches import Ellipse

rc('font',family='roman')
fill((7,9,9,7),(80,80,90,90),'k')
Ellipse((4,80),10,2,facecolor='k',alpha=0.5)
#plot([9,6],[80,40],'--')        # Draw a dashed line of normal thickness
#scatter([2],[20],'+')              # Draw a plus sign
#scatter([3],[20],'p')              # Draw an asterisks
#scatter([4],[20],'o')              # Draw an open circle
#scatter([5],[20],'x')              # Draw an x
#scatter([6],[20],'s')              # Draw a square
#scatter([7],[20],'^')              # Draw a triangle
#scatter([8],[20],'8')             # Draw an open plus sign
axis((0,10,0,100))
savefig('plot-ex1a.eps')

#viewport(0.2,0.8,0.3,0.9)             # Set the location of the window
#default(size=1.2)                     # Set the character size
#lims = [0,10,0,100]                   # Set the world coordinate limits
#rect(7,9,80,90,limits=lims)           # Draw a filled rectangle
#arc(4,80,2,10,fill='h')               # Draw a ellipse
#arrow([5,2],[80,40],style='-',width=2) # Draw a thick solid line
#plot([9,6],[80,40],style='--')        # Draw a dashed line of normal thickness
#plot([2],[20],style='+')              # Draw a plus sign
#plot([3],[20],style='*')              # Draw an asterisks
#plot([4],[20],style='o')              # Draw an open circle
#plot([5],[20],style='x')              # Draw an x
#plot([6],[20],style='s')              # Draw a square
#plot([7],[20],style='^')              # Draw a triangle
#plot([8],[20],style='o+')             # Draw an open plus sign
#axis(verticaly=True)                  # Draw a box with numbers and tick marks
#savefig('plot-ex1.ps')                # Save plot to a file
