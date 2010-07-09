#!/usr/bin/env python
# A simple example showing how to draw histograms.
from pywip import *

viewport(0.2,0.8,0.3,0.8)          # Set the viewport
default(size=1)
# Plot data from 'plotdata2.dat' with x data = column 1, y data = col. 2
bin(1,2,'plotdata2.dat')
axis()                       # Draw a box with tick marks and numbers
xlabel('x')                  # Set label for x axis
ylabel('y')                  # Set label for y axis
title('A Simple Histogram')  # Give plot a title
savefig('plot-ex8.ps')       # save plot to a file
