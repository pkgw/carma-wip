#!/usr/bin/env python
# A plot to illustrate the different font styles and ways of presenting
# text
from pywip import *

viewport(0.2,0.8,0.2,0.9)
lims = [0,1,16,0]
plot([0.5],[12],style='*',limits=lims)
text(0.05,1,r'\sf{Normal:  ABCDQ efgh 1234 \alpha\beta\gamma\delta \Lambda\Theta\Delta\Omega}')
text(0.05,2,r'Roman:  ABCDQ efgh 1234 \alpha\beta\gamma\delta \Lambda\Theta\Delta\Omega')
text(0.05,3,r'\it{Italic:  ABCDQ efgh 1234 \alpha\beta\gamma\delta \Lambda\Theta\Delta\Omega}')
text(0.05,4,r'\cu{Script:  ABCDQ efgh 1234 \alpha\beta\gamma\delta \Lambda\Theta\Delta\Omega}')
text(0.05,5,r'\it{f}(\it{x}) = \it{x}^{2}cos(2\pi\it{x})e^{\it{x}^{2}}')
text(0.05,6,r'\it{H_{0}} = 75 \pm 25 km s^{-1} Mpc^{-1}')
text(0.05,7,r'\cu{L/L_{\odot}} = 5.6 (\lambda1216\AA)')
text(0.05,10,'Bigger (1.5)',size=1.5,width=3)
text(0.5,10,'Smaller (0.5)',size=0.5)
text(0.5,12,'Left justified',align='left')
plot([0.5],[13],style='*')
text(0.5,13,'Centered',align='center')
plot([0.5],[14],style='*')
text(0.5,14,'Right justified',align='right')
plot([0.5],[15],style='*')
text(0.5,15,r'Angle = 45^{o}',angle=45)
axis(number=[],drawtickx=False,drawticky=False)
savefig('plot-ex3.ps')
