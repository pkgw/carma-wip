#!/usr/bin/env python
# A plot to show the different symbols used for plotting points.
# red labels are symbols that can be filled.
from pywip import *
from pywip import _symbols as pysym

def dopanel(symbol,idx):
   '''Plot the given symbol in the given panel'''
   if idx == 1:
      panel(idx,nx=5,ny=3,gapx=0,gapy=0)
   else:
      panel(idx)
   if symbol in ['s','o','^','st']: # can be filled
      clr = 'r'
   else:
      clr = 'k'
   plot([0.5],[0.5],style=symbol,limits=(0,1,0,1),color=clr,size=2)
   text(0.5,0.8,r'%s' %symbol,align='center',color=clr)
   axis(number=[])

winadj(5,3)
title('Symbols in red can be filled',offset=-1)
for i in range(len(pysym)):
   dopanel(pysym[i],i+1)
savefig('plot-ex7.ps')
