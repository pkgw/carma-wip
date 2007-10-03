#!/usr/bin/env python

from pywip import *

viewport(0.2,0.8,0.3,0.9)             # Set the location of the window
default(size=1.2)                     # Set the character size
lims = [0,10,0,100]                   # Set the world coordinate limits
rect(7,9,80,90,fill='s',limits=lims)  # Draw a filled rectangle
rect(3,5,80,90,fill='h')              # Draw a hollow rectangle
plot([5,2],[80,40],style='-',width=2) # Draw a thick solid line
plot([9,6],[80,40],style='--')        # Draw a dashed line of normal thickness
plot([2],[20],style='+')              # Draw a plus sign
plot([3],[20],style='*')              # Draw an asterisks
plot([4],[20],style='o')              # Draw an open circle
plot([5],[20],style='x')              # Draw an x
plot([6],[20],style='s')              # Draw a square
plot([7],[20],style='^')              # Draw a triangle
plot([8],[20],style='o+')             # Draw an open plus sign
axis('bcnst','bcnstv')                # Draw a box with numbers and tick marks
savefig('plot-ex1.ps')                # Save plot to a file
