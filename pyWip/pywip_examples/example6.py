#!/usr/bin/env python
# A plot showing how to generate data internally using python
from pywip import *
import math

def generate(num):
   x = [0]*num
   y = [0]*num
   for i in range(num):
      x[i] = 4*math.pi*(((2*(i+1))-num)/(num - 1.0))
      y[i] = math.cos(abs(x[i]))/(1.0+abs(x[i]))
   return x,y

viewport(0.25,0.85,0.3,0.8)
default(size=1.2)
lims = (-4*math.pi,4*math.pi,-0.5,1.2)
x,y = generate(100)
plot(x,y,style='-',limits=lims)
plot(x,y,style='+')
title(r'y = cos(|x|)/(1+|x|)',size=1.5)
xlabel('x')
ylabel('y')
axis(verticaly=True)
savefig('plot-ex6.ps')
