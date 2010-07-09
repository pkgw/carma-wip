#!/usr/bin/env python
# A plot to illustrate reading data from a file and plotting it.
from pywip import *

viewport(0.2,0.8,0.3,0.8)          # Set the viewport
default(size=1)
# Plot data from 'plotdata.dat' with x data = column 1, y data = col. 2,
# and symbol data for each point from column 3
plot(1,2,'plotdata.dat',style=3,limits=[0,10,0,20])
plot(1,2,'plotdata.dat',style='-') # Connect the data points with a line
axis()                             # Draw a box with tick marks and numbers
xlabel('(x)')                      # Set label for x axis
ylabel('(y)')                      # Set label for y axis
title('A Simple Graph')            # Give plot a title
savefig('plot-ex2.ps')             # save plot to a file
