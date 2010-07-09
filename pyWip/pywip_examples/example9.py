#!/usr/bin/env python
# A simple plot to show off the legend box command and logarithmic axes
#
# Note that if y contains any zeros, than attempting to make a logarithmic
# plot will give you a really strange result without any error message.

from pywip import *

viewport(0.15,0.95,0.15,0.95)
winadj() # make square
default(size=1.5)
plot(1,2,'plotdata.dat',style='-',text='Model',logy=True)
plot(1,2,'ploterror.dat',style='o',text='Real Data',size=1.5)
axis()
legend(0.05,0.95)
xlabel('\it{x}',offset=-1)
ylabel('\it{y}')

savefig('plot-ex9.ps')
