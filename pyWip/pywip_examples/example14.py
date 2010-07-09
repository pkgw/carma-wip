#!/usr/bin/env python
# A plot to illustrate the different palletes available

from pywip import *
from pywip import _palettes as plist

def dolabel(idx):
   '''Print out the label for the palette'''
   x1 = 0.19
   x2 = 0.81
   if idx > 5:
      y = 0.94 - (idx - 6)*0.18
   else:
      y = 0.94 - idx*0.18
   if   idx ==  0: text(x1,y,plist[i],align='center')
   elif idx ==  1: text(x1,y,plist[i],align='center')
   elif idx ==  2: text(x1,y,plist[i],align='center')
   elif idx ==  3: text(x1,y,plist[i],align='center')
   elif idx ==  4: text(x1,y,plist[i],align='center')
   elif idx ==  5: text(x1,y,plist[i],align='center')
   elif idx ==  6: text(x2,y,plist[i],align='center')
   elif idx ==  7: text(x2,y,plist[i],align='center')
   elif idx ==  8: text(x2,y,plist[i],align='center')
   elif idx ==  9: text(x2,y,plist[i],align='center')
   elif idx == 10: text(x2,y,plist[i],align='center')
   elif idx == 11: text(x2,y,plist[i],align='center')

default(xmargin=2.5,ymargin=0.25) # fiddle with spaces between panels
img = 'orion.fits' # image we will plot
for i in range(len(plist)): # put name of each palette before we start
   dolabel(i)               # messing with coordinates by plotting images
plot([0.5,0.5],[0,1],style='-',width=2) # draw a line separating left/right

n = len(plist)/2 # half of the total number of palettes in each viewport
lims = (0,20) # halftone limits
viewport(0,0.5,0.05,0.95) # first viewport
title('Positive                Negative',offset=-1)

for i in range(n): # put positive palettes
   if i == 0:
      panel(2*i+1,nx=2,ny=n,gapx=2.5,gapy=0.25)
   else:
      panel(2*i+1)
   winadj(image=img)
   halftone(img,palette=plist[i],minmax=lims)
   axis(number=[],drawtickx=False,drawticky=False)
   wedge('right',0,2,number=[],majortickx=False,majorticky=False)
for i in range(n): # put negative palettes
   panel(2*i+2)
   winadj(image=img)
   halftone(img,palette='-%s' %plist[i],minmax=lims)
   axis(number=[],drawtickx=False,drawticky=False)
   wedge('left',0,2,number=[],majortickx=False,majorticky=False)

panel(1,nx=1,ny=1) # reset panel numbering for a new viewport
viewport(0.5,1,0.05,0.95) # second viewport
title('Positive                Negative',offset=-1)

for i in range(n): # positive palettes
   if i == 0:
      panel(2*i+1,nx=2,ny=n,gapx=2.5,gapy=0.25)
   else:
      panel(2*i+1)
   winadj(image=img)
   halftone(img,palette=plist[n+i],minmax=lims)
   axis(number=[],drawtickx=False,drawticky=False)
   wedge('right',0,2,number=[],majortickx=True,majorticky=True)
for i in range(n): # negative palettes
   panel(2*i+2)
   winadj(image=img)
   halftone(img,palette='-%s' %plist[n+i],minmax=lims)
   axis(number=[],drawtickx=False,drawticky=False)
   wedge('left',0,2,number=[],majortickx=False,majorticky=False)

savefig('plot-ex14.ps')
