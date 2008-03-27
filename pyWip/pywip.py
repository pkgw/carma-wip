#!/usr/bin/env python
r'''Library functions for running WIP within python
Started 12 April 2007 - NLC
First used for a real plot on 19 April 2007
last updated: 13 March 2008 - NLC

    Color Table                        Palettes
======================     ===============================
| key | Description  |     |   key   |    Description    |
----------------------     -------------------------------
|  w  |     white    |     |   bw    |  black and white  |
|  k  |     black    |     |  gray   |     grayscale     |
|  r  |      red     |     | rainbow |   Rainbow scale   |
|  g  |    green     |     |  heat   |     Heat scale    |
|  b  |     blue     |     |  iraf   |     IRAF scale    |
|  c  |     cyan     |     |  aips   |     AIPS scale    |
|  m  |    magenta   |     | pgplot  |    PGPLOT scale   |
|  y  |    yellow    |     |    a    | SAOimage A scale  |
|  o  |    orange    |     |   bb    | SAOimage BB scale |
| gy  | green-yellow |     |   he    | SAOimage HE scale |
| gc  |  green-cyan  |     |   i8    | SAOimage I8 scale |
| bc  |  blue-cyan   |     |   ds    |      DS scale     |
| bm  | blue-magenta |     | cyclic  |    Cyclic scale   |
| rm  | red-magenta  |     -------------------------------
| dg  |  dark gray   |     * can prepend palette keywords with
| lg  |  light gray  |       a minus sign to reverse the scale
----------------------

    Fill Styles                     Fonts
=======================   =========================
| key |  Description  |   | key |   Description   |
-----------------------   -------------------------
|  s  |    solid      |   |  sf |   sans-serif    |
|  h  |    hollow     |   |  rm | roman (default) |
|  /  |    hatched    |   |  it |     italics     |
|  #  | cross-hatched |   |  cu |     cursive     |
-----------------------   -------------------------

             Symbols                               Line Styles
==================================        ===========================
|  key  |       Description      |        | key  |    Description   |
----------------------------------        ---------------------------
|   s   |         square         |        |   -  |       solid      |
|   .   |           dot          |        |  --  |      dashed      |
|   +   |       plus sign        |        |  .-  |    dot-dashed    |
|   *   |       asterisks        |        |   :  |      dotted      |
|   o   |         circle         |        | -... | dash-dot-dot-dot |
|   x   |          an x          |        ---------------------------
|   ^   |        triangle        |
| oplus |  circle with plus sign |
| odot  |    circle with a dot   |
|  ps   |     pointed square     |
|   d   |        diamond         |
|  st   |    five-point star     |
|  o+   |    open plus symbol    |
| david |     star of david      |
----------------------------------

LaTeX is supported in text strings by default.  Note that unlike math mode in
LaTeX, the text is NOT italicized by  default.  LaTeX symbols supported: all
greek symbols, \times, \AA (for angstroms), \odot (for sun symbol),
\oplus (for earth symbol), \pm (for proper +-), \geq (for greater than or
equal to), \leq (for less than or equal to), ^{} (for superscripts),
_{} (for subscripts), \sf{} (for sans-serif font), \rm{} (for roman font),
\it{} (for italics), \cu{} (for cursive)'''

import os,sys,tempfile,string,math

_palettes = ('bw','gray','rainbow','heat','iraf','aips','pgplot','a','bb',
   'he','i8','ds','cyclic')

_colors = ('w','k','r','g','b','c','m','y','o','gy','gc','bc','bm','rm','dg','lg')

_fills = ('s','h','/','#')

_fonts = ('sf','rm','it','cu')

_lstyles = ('-','--','.-',':','-...')

# These variables all start with a sensible value, however they are actually
# set by the _setglobaldefaults() function.  This function is called whenever
# the _wipopen() function is called, which is whenever the user issues a plot
# command, or whenever savefig() is called.

_wipfile   = '???' # the temporary wip file that holds all the plot commands
_tmplist   = []    # list of any temp files we have made
_panellist = []    # list of dictionaries containing info about limits and
                   # logx/y for each panel
_curpanel  = 0     # index of current panel number in _panellist
_font      = '2'   # default font (roman)
_lwidth    = '1'   # default line width
_lstyle    = '1'   # default line style (solid)
_color     = '1'   # default color (black)
_size      = '1'   # default size
_bg        = '-1'  # default background text color, i.e. transparent

def arc(x,y,majorx,majory,deg=360,start=0,angle=0,fill='s',color=None,style=None,
   width=None,limits=None,logx=None,logy=None):
   '''Draw a curved line to make circles, ellipses, arcs, etc.

      x,y           - central coordinates for arc.  If a string, convert WCS
                      into units usable by WIP.  The WCS can be given as
                      hh:mm:ss, dd:mm:ss or as degrees.  Note that you don't
                      have to give all three parts, you can omit the ss.  x is
                      always assumed to be RA and y is always assumed to be DEC.
      majorx,majory - major axes for x and y.  Like x,y this can be given as
                      a string to specify the WCS.
      deg           - draw arc over this many degrees
      start         - start drawing at this degree angle measured counter-
                      clockwise from +x axis.
      angle         - Tilt angle for curve in degrees measured counter-clockwise
                      from +x axis.
      fill          - a string specifying the fill style
      color         - a string with the color of the line
      style         - a string specifying the line style
      width         - thickness of lines
      limits        - If a list/tuple, use as limits.  Otherwise try to use
                      any prexisting limits or set new ones
      logx          - If True, make logarithmic in x direction.  Otherwise
                      default to what has already been set for plot/panel.
      logy          - If True, make logarithmic in y direction.  Otherwise
                      default to what has already been set for plot/panel'''
   fp = _wipopen()
   _updatepanel(limits,logx,logy)
   _writelimits(fp)
   _writeoptions(fp,color,None,None,style,width)
   fillstyle = _translatefill(fill)
   fp.write('fill %s\n' %fillstyle)
   fp.write('move %f %f\n' %(_translatecoords(x,coord='ra'),_translatecoords(y,coord='dec')))
   fp.write('angle %f\n' %angle)
   tmpx = _translatecoords(majorx,coord='ra')
   tmpy = _translatecoords(majory,coord='dec')
   fp.write('arc %f %f %f %f\n' %(tmpx,tmpy,deg,start))
   _resetdefaults(fp,color,None,None,style,width)
   fp.write('angle 0\n')
   _updatepanel(limits,logx,logy,overlap=True)
   fp.close()

def axis(xaxis=None,yaxis=None,color=None,font=None,size=None,style=None,
   width=None,xtick=None,ytick=None):
   '''Draw the axes.

      xaxis,yaxis - wip string for drawing x and y axis.  If left as None,
                    then axis function will try to guess something appropriate
                    based on panel arrangement and whether log of x or y has
                    been taken.  If an image has been plotted, then axis will
                    assume you want WCS labeling.
      color       - the color of the axis as a string
      font        - the font to use for the axis
      size        - the size for numbers on axis
      style       - the line style
      width       - the thickness of the lines
      xtick       - tuple of two numbers.  First value is interval between major
                    tick marks in x.  Second value is number of minor tick marks
                    between each major tick mark.
      ytick       - Same as xtick except for y axis'''
      
   nx = _panellist[_curpanel]['nx'] # number of panels in x and y directions
   ny = _panellist[_curpanel]['ny'] # used for automatically labelling axes
   image = _panellist[_curpanel]['image'] # Name of image in panel 
   fp = _wipopen()
   _writeoptions(fp,color,font,size,style,width)
   if _isseq(xtick) and _isseq(ytick):
      if len(xtick) == 2 and len(ytick) == 2:
         fp.write('ticksize %f %f %f %f\n' %(xtick[0],xtick[1],ytick[0],ytick[1]))
      else:
         _error('You must have two arguments to xtick and ytick!')
   elif _isseq(xtick):
      if len(xtick) == 2:
         fp.write('ticksize %f %f 0 0\n' %(xtick[0],xtick[1]))
      else:
         _error('You must have two arguments to xtick!')
   elif _isseq(ytick):
      if len(ytick) == 2:
         fp.write('ticksize 0 0 %f %f\n' %(ytick[0],ytick[1]))
      else:
         _error('You must have two arguments to ytick!')
   if not xaxis:
      if image: # If an image has been defined, assume user wants WCS
         xlab = 'bcnsthz'
      else:
         xlab = 'bcst'
         if ny > 0 or _curpanel/abs(nx) == (abs(ny) - 1): # space between panels
            xlab = 'bcnst'                                # or on last row 
         if _panellist[_curpanel]['logx']:
            xlab = xlab + 'l'
   else:
      xlab = xaxis
   if not yaxis:
      if image: # If an image has been defined, assume user wants WCS
         ylab = 'bcnstvdyz'
      else:
         ylab = 'bcst'
         if nx > 0 or _curpanel%abs(nx) == 0: # space between panels
            ylab = 'bcnst'                    # or on first column 
         if _panellist[_curpanel]['logy']:
            ylab = ylab + 'l'
   else:
      ylab = yaxis
   fp.write('box %s %s\n' %(xlab,ylab))
   _resetdefaults(fp,color,font,size,style,width)
   if _isseq(xtick) or _isseq(ytick):
      fp.write('ticksize 0 0 0 0\n')
   fp.close()

def beam(x,y,amajor,aminor,angle=0,scale='ra',color=None,fillcolor=None,
   style=None,width=None,bg=None):
   '''Draw an ellipse showing a beam.  Useful for radio data.

      x,y           - Central coordinates for beam.  If a string, convert WCS
                      into units usable by WIP.  The WCS can be given as
                      hh:mm:ss, dd:mm:ss or as degrees.
      amajor,aminor - major and minor axes for ellipse as arcseconds. Like x,y
                      this can be given as a string to specify the WCS.
      angle         - tilt angle for ellipse given as degrees from the +x axis
      scale         - set to None for no scaling of x axis
      color         - color of edge of ellipse
      fillcolor     - color of inside of ellipse.  Defaults to color
      style         - line style for edge of ellipse
      width         - line thickness
      bg            - color for background box surrounding beam.  Defaults to
                      transparent.'''
   fp = _wipopen()
   if scale == 'ra':
      scl = '-1'
   else:
      scl = '1'
   if bg:
      bgrect = _translatecolor(bg)
   else:
      bgrect = '-1'
   if fillcolor:
      fillcl = _translatecolor(fillcolor)
   else:
      if color:
         fillcl = _translatecolor(color)
      else:
         fillcl = _color
   _writeoptions(fp,color,None,None,style,width)
   fp.write('move %f %f\n' %(_translatecoords(x,coord='ra'),_translatecoords(y,coord='dec')))
   tmpx = _translatecoords(amajor,coord='ra')
   tmpy = _translatecoords(aminor,coord='dec')
   fp.write('beam %f %f %f 0 0 %s %s %s\n' %(tmpx,tmpy,angle,scl,fillcl,bgrect))
   _resetdefaults(fp,color,None,None,style,width)
   fp.close()

def bin(xcol,ycol,datafile=None,color=None,width=None,style=None,limits=None,
   coord='center',logx=False,logy=False,text=None):
   '''Draw a histogram from previously histogrammed data.

      xcol,ycol - either an integer specifying column in datafile or a
                  list/tuple of x/y data points
      datafile  - string containing filename with data.  Leave as None if
                  xcol and ycol are tuples/lists
      color     - the color for the histogram
      width     - the thickness of the histogram
      style     - line style for histogram.  Defaults to solid
      limits    - If a list/tuple, use as limits.  Otherwise try to use
                  any prexisting limits or set new ones
      coord     - are x coordinates the center or left side of the bin?
      logx      - If True, make logarithmic in x direction.  Otherwise
                  default to what has already been set for plot/panel.
      logy      - If True, make logarithmic in y direction.  Otherwise
                  default to what has already been set for plot/panel
      text      - Text to be used for curve.  Defaults to "Generic Curve"'''
      
   if style:
      plotcurve = curve(color,None,style,width,text)
   else: # default style should be a solid line
      plotcurve = curve(color,None,'-',width,text)
   fp = _wipopen()
   pan = _updatepanel(limits,logx,logy)
   if coord == 'center':
      k = 1
   elif coord == 'left':
      k = 0
   else:
      _error('Invalid coord parameter in bin function.  Try center or left.')

   if not datafile:
      datafile = _maketempfile(xcol,ycol)
      xcol = 1
      ycol = 2

   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   if _panellist[_curpanel]['logx']:
      fp.write('log x\n')
   if _panellist[_curpanel]['logy']:
      fp.write('log y\n')
   _writelimits(fp)
   _writeoptions(fp,color,None,None,style,width)
   fp.write('bin %d\n' %k)
   _resetdefaults(fp,color,None,None,style,width)
   _updatepanel(limits,logx,logy,overlap=True)
   fp.close()
   return plotcurve

def blowup(xmin,xmax,ymin,ymax,corners=['ll','ul'],color=None,style=None,
   width=None):
   r'''Draw a blow-up box around a given region.

      Draws a blow-up box around a region and also stores the region limits in
      wip registers \9 - \12.  These are used by the connect command to draw
      connecting lines from the blowup box to the actual zoomed region.

      xmin,xmax,ymin,ymax - four values for limits of blowup box given as
                            pixel values
      corners - a two element list of ll,ul,lr, or ur to specify which corners
                of the box to store in memory.  These correspond to lower left,
                upper left,lower right, upper right
      color   - a string giving the color for the box
      style   - a string giving the linestyle for the box
      width   - a number giving the thickness of the line'''

   fp = _wipopen()
   _writeoptions(fp,color,None,None,style,width)
   fp.write('limits 1 \\7 1 \\8\n') # ensures that xmin,xmax,ymin,ymax are all
                                    # pixel coordinates.  Assumes the user has
                                    # set \7 and \8 through winadj
   fp.write('fill %s\n' %_translatefill('h')) # always hollow
   fp.write('rect %f %f %f %f\n' %(xmin,xmax,ymin,ymax))
   if corners[0] == 'll':
      _xytovp(fp,str(xmin),str(ymin),r'\9',r'\10')
   elif corners[0] == 'ul':
      _xytovp(fp,str(xmin),str(ymax),r'\9',r'\10')
   elif corners[0] == 'lr':
      _xytovp(fp,str(xmax),str(ymin),r'\9',r'\10')
   elif corners[0] == 'ur':
      _xytovp(fp,str(xmax),str(ymax),r'\9',r'\10')
   if corners[1] == 'll':
      _xytovp(fp,str(xmin),str(ymin),r'\11',r'\12')
   elif corners[1] == 'ul':
      _xytovp(fp,str(xmin),str(ymax),r'\11',r'\12')
   elif corners[1] == 'lr':
      _xytovp(fp,str(xmax),str(ymin),r'\11',r'\12')
   elif corners[1] == 'ur':
      _xytovp(fp,str(xmax),str(ymax),r'\11',r'\12')
   _resetdefaults(fp,color,None,None,style,width)
   fp.close()

def connect(corners=['ll','ul'],color=None,style=None,width=None):
   '''Draw lines connecting a blow-up region to the current viewport coordinates.

      corners - a two element list of ll,ul,lr, or ur to specify which corners
                of the box to store in memory.  These correspond to lower left,
                upper left,lower right, upper right
      color   - a string giving the color for the box
      style   - a string giving the linestyle for the box
      width   - a number giving the thickness of the line'''

   fp = _wipopen()
   _writeoptions(fp,color,None,None,style,width)
   fp.write('new tmp1 tmp2 tmp3 tmp4 xp yp\n')
   if corners[0] == 'll':
      _xytovp(fp,'x1','y1','tmp1','tmp2')
   elif corners[0] == 'ul':
      _xytovp(fp,'x1','y2','tmp1','tmp2')
   elif corners[0] == 'lr':
      _xytovp(fp,'x2','y1','tmp1','tmp2')
   elif corners[0] == 'ur':
      _xytovp(fp,'x2','y2','tmp1','tmp2')
   if corners[1] == 'll':
      _xytovp(fp,'x1','y1','tmp3','tmp4')
   elif corners[1] == 'ul':
      _xytovp(fp,'x1','y2','tmp3','tmp4')
   elif corners[1] == 'lr':
      _xytovp(fp,'x2','y1','tmp3','tmp4')
   elif corners[1] == 'ur':
      _xytovp(fp,'x2','y2','tmp3','tmp4')
   fp.write('viewport 0 1 0 1\n')
   _vptoxy(fp,r'\9',r'\10','xp','yp')
   fp.write('move xp yp\n')
   _vptoxy(fp,'tmp1','tmp2','xp','yp')
   fp.write('draw xp yp\n')
   _vptoxy(fp,r'\11',r'\12','xp','yp')
   fp.write('move xp yp\n')
   _vptoxy(fp,'tmp3','tmp4','xp','yp')
   fp.write('draw xp yp\n')
   fp.write('free tmp1 tmp2 tmp3 tmp4 xp yp\n')
   _resetdefaults(fp,color,None,None,style,width)
   fp.close()

def contour(image,header,levels,routine='smooth',limits=None,color=None,
   font=None,style=None,width=None,border=False):
   r'''Draw contours with the specified levels.

      image   - string with name of image
      header  - string with header for image such as rd, px, etc.
      levels  - a list/tuple or string of contour levels.  If a string,
                give as min:max:step to specify levels, or as 'border', if
                you just want to draw a box around the border of the image.
      routine - how to draw contours: smooth, fast, neg.  neg will draw negative
                contours with the same line style as positive ones.  By default,
                negative contours are drawn dashed.
      limits  - If not specified show whole image.  If a list/tuple of
                four numbers, plot given sub-image.  If anything else,
                use limits stored in \1 \2 \3 \4 (set by halftone)
      color   - color for contour lines
      font    - font for contour labelling (TODO: not supported)
      style   - line style for contours
      width   - thickness of contour lines'''
   fp = _wipopen()
   borderFlag = False # set to true if we want to draw the border instead of
                      # contours
   if _isstr(levels) and levels.lower() == 'border':
      borderFlag = True
   
   if not _panellist[_curpanel]['image']:
      _readimage(fp,image)
   elif _panellist[_curpanel]['image'] != image:
      _readimage(fp,image)
   fp.write('header %s\n' %header)
   if borderFlag:
      fp.write('set \\13 x1\n')
      fp.write('set \\14 x2\n')
      fp.write('set \\15 y1\n')
      fp.write('set \\16 y2\n')
   else:
      fp.write('levels %s\n' %_translatelevels(levels))
   _writeimglimits(fp,limits)
   _writeoptions(fp,color,font,None,style,width)
   if borderFlag:
      fp.write('move \\13 \\15\n')
      fp.write('draw \\14 \\15\n')
      fp.write('draw \\14 \\16\n')
      fp.write('draw \\13 \\16\n')
      fp.write('draw \\13 \\15\n')
   else:
      if style == '--': # must set to fast to get dashed contours to show up
         routine = 'fast'
      if routine == 'smooth':
         fp.write('contour t\n')
      elif routine == 'fast':
         fp.write('contour s\n')
      elif routine == 'neg': # draw negative contours with same lstyle as + ones
         fp.write('contour -t\n')
      else:
         _error('Invalid routine keyword.  Try fast, smooth, or neg!')
   _resetdefaults(fp,color,font,None,style,width)
   fp.close()
   _updatepanel(limits,None,None,image,overlap=True)

def curve(color=None,size=None,style=None,width=None,text=None,fillcolor=None):
   '''Return a generic curve that can be used by a legend.

      A curve is a dictionary that is returned by the plot command and can
      be used with the legend command to make it easier to make legends.  The
      keyword arguments give the default values for the dictionary.'''
      
   c = {'color' : _colors[int(_color)], 'size' : _size, 'style' : 'o', 
      'width' : _lwidth, 'text' : 'Generic Curve', 'fillcolor' : _bg, 
      'fillsize' : _size, 'fillstyle' :_lstyle}
   if color:
      c['color'] = color
   if size:
      c['size'] = str(size)
   if style:
      c['style'] = style
   if width:
      c['width'] = str(width)
   if text:
      c['text'] = _translatelatex(text)
   if fillcolor:
      fillstyle,fillfactor = _translatefillsymbol(style)
      c['fillcolor'] = fillcolor
      c['fillsize']  = fillfactor*float(c['size'])
      c['fillstyle'] = fillstyle
   return c

def default(color=None,font=None,size=None,style=None,width=None,bg=None,
   xmargin=None,ymargin=None):
   '''Set default parameters that will be used for everything in the plot.

      color  - a string giving the default color
      font   - a string giving the default font
      size   - a number giving the default size for everything
      style  - a string giving the default line style
      width  - a number giving the default width of lines
      bg     - a string giving the background color for text.  Can set to -1
               for transparent (the default)
      xmargin - set space between panels to this number of character units
                in the x direction.  Default is 2.
      ymargin - set space between panels to this number of character units
                in the y direction.  Default is 2.'''
   global _color, _lwidth, _font, _size, _lstyle, _bg
   if color:
      _color = _translatecolor(color)
   if font:
      _font  = _translatefont(font)
   if size:
      _size = str(size)
   if style:
      _lstyle = _translatelstyle(style)
   if width:
      _lwidth = str(width)
   if bg:
      if bg == '-1':
         _bg = '-1'
      else:
         _bg = _translatecolor(bg)
   if xmargin:
      fp = _wipopen()
      fp.write('set xsubmar %f\n' %xmargin)
      fp.close()
   if ymargin:
      fp = _wipopen()
      fp.write('set ysubmar %f\n' %ymargin)
      fp.close()

def errorbar(xcol,ycol,datafile=None,xerr=None,yerr=None,color=None,size=None,
   style=None,width=None,limits=None,logx=False,logy=False):
   '''Draw x,y errorbars on some data.

      xcol,ycol - Integer or list/tuple of x/y data
      datafile  - If a string, read columns xcol and ycol from datafile
      xerr      - Integer or list/tuple of x errorbars.  If int, read specified
                  column from datafile
      yerr      - Integer or list/tuple of y errorbars.  If int, read specified
                  column from datafile
      color     - A string specifying the color of the errorbars
      size      - A number specifying the size of the caps on the errorbars
      style     - A string specifying the line style
      width     - A number specifying the line thickness
      limits    - If a list/tuple, use as limits.  Otherwise try to use
                  any prexisting limits or set new ones
      logx,logy - If True, plot logarithmic errorbars'''

   pan = _updatepanel(limits,logx,logy)
   ecol = 3 # column number for errorbar
   if not datafile:
      if _isseq(xcol) and _isseq(ycol):
         datafile = _maketempfile(xcol,ycol,xerr,yerr)
      else:
         _error('You must specify an input data file for errorbar!')
   else:
      datafile = _maketempfile2(xcol,ycol,datafile,xerr,yerr)

   fp = _wipopen()
   _writeoptions(fp,color,None,size,style,width)
   fp.write('data %s\n' %datafile)
   fp.write('xcol 1\n')
   fp.write('ycol 2\n')
   if pan['logx']:
      fp.write('log x\n')
   if pan['logy']:
      fp.write('log y\n')
   if xerr:
      fp.write('ecol %d\n' %ecol)
      if pan['logx']:
         _writelimits(fp)
         fp.write('log err\n')
         fp.write('errorbar 1\n')
         ecol = ecol + 1
         fp.write('ecol %d\n' %ecol)
         fp.write('log err\n')
         fp.write('errorbar 3\n')
      else:
         fp.write('errorbar 5\n')
      ecol = ecol + 1
   if yerr:
      fp.write('ecol %d\n' %ecol)
      if pan['logy']:
         fp.write('log err\n')
         fp.write('errorbar 2\n')
         ecol = ecol + 1
         fp.write('ecol %d\n' %ecol)
         fp.write('log err\n')
         fp.write('errorbar 4\n')
      else:
         fp.write('errorbar 6\n')
   _resetdefaults(fp,color,None,size,style,width)
   fp.close()

def halftone(image,header='px',palette='gray',minmax=None,blank=None,
   limits=None):
   r'''Plot a halftone image with specified palette.

      Before using this, one should call winadj to make sure your pixels come
      out square.

      image   - string with name of image
      header  - string of header for image such as rd, px, etc.
      palette - color palette for halftone
      minmax  - list or tuple of (min,max) halftone limits to display
      blank   - If set, ignore specified value when making halftone
      limits  - If not specified show whole image.  If a list/tuple of
                four numbers, plot given sub-image.  If anything else,
                use limits stored in \1 \2 \3 \4 (set by halftone)'''

   fp = _wipopen()
   pal = _translatepalette(palette)
   if not _panellist[_curpanel]['image']:
      _readimage(fp,image)
   elif _panellist[_curpanel]['image'] != image:
      _readimage(fp,image)
   fp.write('header %s\n' %header)
   fp.write('palette %s\n' %pal)
   _writeimglimits(fp,limits)
   fp.write('halftone ')
   if minmax:
      fp.write('%f %f ' %tuple(minmax))
   if blank:
      fp.write('0 %f' %blank)
   fp.write('\n')
   fp.write('set \\1 x1\n') # store map limits in registers 1-4
   fp.write('set \\2 x2\n') # useful for contour overlays
   fp.write('set \\3 y1\n')
   fp.write('set \\4 y2\n')
   if minmax:
      fp.write('set \\5 %f\n' %minmax[0])
      fp.write('set \\6 %f\n' %minmax[1])
   else:
      fp.write('set \\5 immin\n')
      fp.write('set \\6 immax\n')
   fp.close()
   _updatepanel(limits,None,None,image,overlap=True)

def legend(x,y,curves,dx,dy,length,height,align='left',color=None,font=None,
   size=None,style=None,width=None,bg=None):
   '''Make an entire legend box.
 
      x,y       - starting x and y location for legend
      curves    - a list/tuple of dictionarys that were returned by the plot
                  or curve functions
      dx        - space between line and text for each entry
      dy        - offset of line from text for each entry
      length    - length of line for each entry
      height    - vertical offset between each entry
      align     - alignment for text.  left, right, or center
      color     - a string giving the color for the text
      font      - a string giving the font to use for the text
      size      - a number giving the size for text
      style     - a string giving the line style for the text
      width     - a number giving the thickness of the line
      bg        - color for background of text.  Default is transparent'''

   al = _translatealign(align)
   fp = _wipopen()
   pan = _updatepanel(None,None,None)
   tmpxlin = _translatecoords(x,'ra')            # starting x coordinate for line
   tmpxsym = _translatecoords(x,'ra')+length/2.0 # starting x coordinate for symbol
   tmpxlen = _translatecoords(x,'ra')+length     # end x coordinate for line   
   tmpylin = _translatecoords(y,'dec')+dy        # y coordinate for line
   tmpxtxt = _translatecoords(x,'ra')+length+dx  # starting x coordinate for text
   tmpytxt = _translatecoords(y,'dec')           # starting y coordinate for text
   tmpskip = _translatecoords(height,'dec')      # vertical skip for each entry in legend

   if pan['logx'] == True:
      tmpxlin = math.log10(tmpxlin)
      tmpxsym = math.log10(tmpxsym)
      tmpxlen = math.log10(tmpxlen)
      tmpxtxt = math.log10(tmpxtxt)
   if pan['logy'] == True:
      tmpylin = math.log10(tmpylin)
      tmpytxt = math.log10(tmpytxt)

   for c in curves: #loop through each curve and put it in the legend
      sym = c['style']
      # First do the symbol or line for the entry
      if sym in _lstyles: # draw a line
         fp.write('move %f %f\n' %(tmpxlin,tmpylin)) # starting x/y coordinates
         _writeoptions(fp,c['color'],None,c['size'],c['style'],c['width'])
         fp.write('draw %f %f\n' %(tmpxlen,tmpylin))
         _resetdefaults(fp,c['color'],None,c['size'],c['style'],c['width'])
      else: # put individual symbol
         fp.write('move %f %f\n' %(tmpxsym,tmpylin))
         _writeoptions(fp,c['color'],None,c['size'],None,c['width'])
         fp.write('symbol %s\n' %_translatesymbol(c['style']))
         fp.write('dot\n')
         _resetdefaults(fp,c['color'],None,c['size'],None,c['width'])
         if c['fillcolor'] != '-1':
            _writeoptions(fp,c['fillcolor'],None,c['fillsize'],None,c['width'])
            fp.write('symbol %s\n' %_translatesymbol(c['fillstyle']))
            fp.write('dot\n')
            _resetdefaults(fp,c['fillcolor'],None,c['fillsize'],None,c['width'])
      # Now do the text label for the entry
      _writeoptions(fp,color,font,size,style,width,bg)
      fp.write('move %f %f\n' %(tmpxtxt,tmpytxt))
      fp.write('putlabel %s %s\n' %(al,c['text']))
      _resetdefaults(fp,color,font,size,style,width,bg)
      tmpylin = tmpylin + tmpskip
      tmpytxt = tmpytxt + tmpskip
   fp.close()

def panel(nx,ny,idx):
   '''Switch to panel idx.

      There are nx,ny panels in a grid.  If nx or ny is negative, then there
      will be no space between panels in that direction.  If idx is negative,
      the panels are counted from the upper left downward.  If positive, the
      panels are counted from the lower left corner upward.

      nx  - Number of panels in x direction.
      ny  - Number of panels in y direction.
      idx - Current panel number.'''

   global _panellist, _curpanel
   fp = _wipopen()
   fp.write('panel %d %d %d\n' %(nx,ny,idx))
   fp.write('color %s\n'  %_color)
   fp.write('font %s\n'   %_font)
   fp.write('expand %s\n' %_size)
   fp.write('lstyle %s\n' %_lstyle)
   fp.write('lwidth %s\n' %_lwidth)
   fp.write('bgci %s\n'   %_bg)
   if abs(nx*ny) > len(_panellist):
      tmp = {'limits' : None, 'logx' : False, 'logy' : False, 'image' : None,
      'overlap' : False, 'nx' : nx, 'ny' : ny}
      for i in range(len(_panellist)): # update existing entries with new panel
         _panellist[i]['nx'] = nx      # configuration
         _panellist[i]['ny'] = ny
      for i in range(abs(nx*ny) - len(_panellist)): # add new entries up to
         _panellist.append(tmp.copy())              # number of panels
   _curpanel = abs(idx) - 1 # set current panel number
   _writelimits(fp)
   fp.close()

def plot(xcol,ycol,datafile=None,color=None,size=None,style='o',width=None,
   fillcolor=None,limits=None,logx=False,logy=False,text='Generic Curve'):
   '''Plot data from a file or input lists/tuples.

      This function is extremely versatile and can be used to plot data
      points and lines.  Note that when specifying column numbers from a file,
      columns are counted starting from 1.
      xcol,ycol - either an integer, list, or tuple for x/y data points.  If
                  integers, these are column numbers to read from the specified
                  datafile.
      datafile  - string name of input data file.  Leave as None if
                  xcol and ycol are a sequence of numbers
      color     - If a string, use as the color for every point.  If an integer,
                  read that column from the datafile for color index for each
                  point.
      size      - The size for each data point.
      style     - If a string, use as the symbol or line style.  If an integer,
                  then read from datafile for symbol for each point.
      width     - Line width
      fillcolor - Color to fill symbols with.  Only available for five-point
                  stars, squares, circles, and triangles.  If used inappropriately,
                  an error will occur.  Use in the same way as for color keyword.
                  Default is 'None' which means no filling will occur (transparent).
      limits    - If None, set min/max to values in xcol and ycol.  If a list
                  or tuple of xmin,xmax,ymin,ymax, set to specified values.
                  If anything else, do not attempt to set any limits (i.e.
                  assume they have already been set to something sensible)
      logx,logy - If True, take log of x/y values before plotting.  Useful for
                  logarithmic plots.  Note that limits must be specified in
                  relation to the logarithms to work properly.
      text      - A string that can be used for the legend command.  Defaults
                  to "Generic Curve"'''

   eFlag = False # Set to true if we are reading a column from the file for
                 # a color for each point
   sFlag = False # Set to true if we are reading a column from the file for
                 # a symbol for each point

   if fillcolor:
      if _isstr(style):
         fillstyle,fillfactor = _translatefillsymbol(style)
      if size:   # fill size must be larger because filled symbols are smaller
         fillsize = fillfactor*size # than regular versions
      else:
         fillsize = fillfactor*float(_size)

   fp = _wipopen()
   pan = _updatepanel(limits,logx,logy)
   plotcurve = curve(color,size,style,width,text,fillcolor)
   if not datafile:
      if _isseq(xcol) and _isseq(ycol):
         num = len(xcol)
         if num > 10:
            datafile = _maketempfile(xcol,ycol)
            xcol = 1
            ycol = 2
         else:
            if fillcolor: # size must be larger because filled versions are 
               _plotpoints(fp,xcol[:],ycol[:],fillcolor,fillsize,fillstyle,width)
            _plotpoints(fp,xcol[:],ycol[:],color,size,style,width)
      else:
         _error('You must specify a datafile for plot!')
   else:
      num = _count(datafile)
   if num == 0: # datafile is empty
      return plotcurve
   if datafile:
      if xcol == 0 or ycol == 0: # make sure user doesn't enter zero-based numbers
         _error('Column numbers start at 1, not zero!')
      fp.write('data %s\n' %datafile)
      fp.write('xcol %s\n' %str(xcol))
      fp.write('ycol %s\n' %str(ycol))
      if pan['logx']:
         fp.write('log x\n')
      if pan['logy']:
         fp.write('log y\n')
      _writeoptions(fp,None,None,size,None,width)
      try: # see if color is an integer, meaning a column in the datafile
         blah = int(color)
         fp.write('ecol %d\n' %blah)
         eFlag = True
      except (ValueError,TypeError):
         _writeoptions(fp,color,None,None,None,None)
      try: # see if style is an integer, meaning a column in the datafile 
         blah = int(style)
         fp.write('pcol %d\n' %blah)
         sym = '1'
         sFlag = True
      except (ValueError,TypeError):
         sym = _translatesymbol(style)
         if sym == '99': # not a symbol so see if it is a line style
            line = _translatelstyle(style)
      _writelimits(fp)
      if sym != '99': # plot a symbol instead of a line
         if not sFlag: # i.e. don't read symbols from datafile
            fp.write('symbol %s\n' %sym)
         if eFlag: # read colors for each point from the file
            fp.write('points 1\n')
         else:
            fp.write('points\n')
         _resetdefaults(fp,color,None,size,None,width)
         if fillcolor:
            fp.write('symbol %s\n' %_translatesymbol(fillstyle))
            try: # see if fillcolor is a column number from the datafile
               blah = int(fillcolor)
               _writeoptions(fp,None,None,fillsize,None,None)
               fp.write('ecol %d\n' %blah)
               fp.write('points 1\n')
            except (ValueError,TypeError):
               _writeoptions(fp,fillcolor,None,fillsize,None,None)
               fp.write('points\n')
            _resetdefaults(fp,fillcolor,None,fillsize,None,None)
      else: # plot a line
         fp.write('lstyle %s\n' %line)
         fp.write('connect\n')
         _resetdefaults(fp,color,None,size,style,width)
   fp.close()
   _updatepanel(limits,logx,logy,overlap=True)
   return plotcurve

def poly(xcol,ycol,datafile=None,color=None,style=None,width=None,fill='s',
   limits=None,logx=False,logy=False):
   '''Draw a closed polygon.

      xcol,ycol - either an integer, list, or tuple for x/y data points
      datafile  - string name of input data file.  Leave as None if
                  xcol and ycol are a sequence of numbers
      color     - string with color to use for line style and fill
      style     - string with line style to draw polygon with
      width     - Line width of polygon
      fill      - Fill style for polygon
      limits    - If None, set min/max to values in xcol and ycol.  If a list
                  or tuple of xmin,xmax,ymin,ymax, set to specified values.
                  If anything else, do not attempt to set any limits (i.e.
                  assume they have already been set to something sensible)
      logx      - If True, make logarithmic in x direction.  Otherwise
                  default to what has already been set for plot/panel.
      logy      - If True, make logarithmic in y direction.  Otherwise
                  default to what has already been set for plot/panel'''

   fp = _wipopen()
   fillstyle = _translatefill(fill)
   _writeoptions(fp,color,None,None,style,width)
   if not datafile:
      datafile = _maketempfile(xcol,ycol)
      xcol = 1
      ycol = 2
   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   fp.write('fill %s\n' %fillstyle)
   _updatepanel(limits,logx,logy)
   _writelimits(fp)
   fp.write('poly\n')
   _resetdefaults(fp,color,None,None,style,width)
   _updatepanel(limits,logx,logy,overlap=True)
   fp.close()

def rect(xmin,xmax,ymin,ymax,color=None,style=None,width=None,fill='s',
   limits=None,logx=False,logy=False):
   '''Draw a rectangle on the figure.

      This function uses the poly command behind the scenes.
      xmin,xmax,ymin,ymax - the limits of the rectangle
      color  - string with color to use for line style and fill
      style  - string with line style to draw rectangle with
      width  - thickness of line used to draw rectangle
      fill   - fill style for rectangle
      limits - If a list/tuple, use as limits.  Otherwise try to use
               any prexisting limits or set new ones
      logx   - If True, make logarithmic in x direction.  Otherwise
               default to what has already been set for plot/panel.
      logy   - If True, make logarithmic in y direction.  Otherwise
               default to what has already been set for plot/panel'''

   poly([xmin,xmax,xmax,xmin],[ymin,ymin,ymax,ymax],color=color,style=style,
      width=width,fill=fill,limits=limits,logx=logx,logy=logy)

def savefig(filename,orient='portrait',color='y',debug=0):
   '''Make the output plot by actually running wip.

      filename - a string or list/tuple giving the output filename(s) with
                 either a .gif or .ps extension
      orient   - make plot portrait or landscape orientation?
      color    - make plot in color or black and white?
      debug    - If 1, do not delete all temp files needed by wip'''

   if orient not in ('landscape','portrait'):
      _error('invalid orient option in savefig.  Try landscape or portrait')
   if color not in ('y','n'):
      _error('Invalid color option in savefig.  Try y or n')

   if _isseq(filename):
      fileseq = filename
   else:
      fileseq = [filename]
   for f in fileseq:
      dev = '???'
      if f[-4:] == '.gif':
         dev = '%s/gif' %f
      elif f[-3:] == '.ps':
         if orient == 'landscape' and color == 'y':
            dev = '%s/cps' %f
         elif orient == 'landscape' and color == 'n':
            dev = '%s/ps' %f
         elif orient == 'portrait' and color == 'y':
            dev = '%s/vcps' %f
         elif orient == 'portrait' and color == 'n':
            dev = '%s/vps' %f
      else:
         _error('Invalid output plot filename suffix.  Try .ps or .gif')
      if dev != '???' and _wipfile != '???':
         os.system('wip -x -d %s %s' %(dev,_wipfile))
   if not debug:
      os.remove(_wipfile)
      for f in _tmplist:
         os.remove(f)
   _setglobaldefaults()

def text(x,y,text,align='left',angle=None,color=None,font=None,size=None,
   style=None,width=None,logx=False,logy=False,bg=None):
   '''Put a given text label at specified coordinates.

      x,y       - coordinates for location of text
      text      - a string of text to print out
      align     - alignment for label.  Either left, center, or right
      angle     - angle is degrees for text
      color     - a string giving the color for the label
      font      - a string giving the font to use for the label
      size      - a number giving the size for label
      style     - a string giving the line style for the label
      width     - a number giving the width of the lines
      logx,logy - If True, take log of x/y coordinates before plotting
      bg        - background color for text.  Default is -1 (transparent)'''
   pan = _updatepanel(None,logx,logy)
   if pan['logx']:
      xtmp = math.log10(_translatecoords(x,coord='ra'))
   else:
      xtmp = _translatecoords(x,coord='ra')
   if pan['logy']:
      ytmp = math.log10(_translatecoords(y,coord='dec'))
   else:
      ytmp = _translatecoords(y,coord='dec')
   fp = _wipopen()

   al = _translatealign(align)
   _writeoptions(fp,color,font,size,style,width,bg)
   if angle:
      fp.write('angle %s\n' %angle)
   fp.write('move %f %f\n' %(xtmp,ytmp))
   fp.write('putlabel %s %s\n' %(al,_translatelatex(text)))
   _resetdefaults(fp,color,font,size,style,width,bg)
   if angle:
      fp.write('angle 0\n')
   fp.close()

def title(text,offset=0,align='center',color=None,font=None,size=None,
   style=None,width=None,bg=None):
   '''Set the title of the plot to the given text.

      text   - a string of text to print out as title of plot
      offset - offset for text in addition to standard offset for a title
      align  - alignment for label.  Either left, center, or right
      color  - a string giving the color for the title
      font   - a string giving the font to use for the title
      size   - a number giving the size for the title
      style  - a string giving the line style for the title
      width  - a number giving the width of the lines
      bg     - background color for text'''
   fp = _wipopen()
   al = _translatealign(align)
   off = str(2.0 + float(offset))
   _resetdefaults(fp,1,1,1,1,1,1) # xlabel doesn't properly pick-up default
                                  # parameters that are set so we force them
                                  # to be written out
   # Now override defaults
   _writeoptions(fp,color,font,size,style,width,bg)
   fp.write('mtext T %s 0.5 %s %s\n' %(off,al,_translatelatex(text)))
   _resetdefaults(fp,color,font,size,style,width,bg)
   fp.close()

def vector(xcol,ycol,anglecol,lengthcol,datafile=None,color=None,size=None,
   width=None,limits=None,logx=None,logy=None):
   '''Draw a vector field.

      pcol is a column with the length of each arrow and ecol is the angle for
      each arrow, in degrees.

      xcol,ycol - either an integer, list, or tuple for x/y data points
      anglecol  - either an integer, list, or tuple for direction of arrows
      lengthcol - either an integer, list, or tuple for length of arrows
      datafile  - string name of input data file.  Leave as None if
                  xcol,ycol,pcol,ecol are a sequence of numbers
      color     - string with color for each arrow
      size      - Size of arrows
      width     - Line width of arrows
      limits    - If None, set min/max to values in xcol and ycol.  If a list
                  or tuple of xmin,xmax,ymin,ymax, set to specified values.
                  If anything else, do not attempt to set any limits (i.e.
                  assume they have already been set to something sensible)
      logx      - If True, make logarithmic in x direction.  Otherwise
                  default to what has already been set for plot/panel.
      logy      - If True, make logarithmic in y direction.  Otherwise
                  default to what has already been set for plot/panel'''

   global _tmplist
   fp = _wipopen()
   pan = _updatepanel(limits,logx,logy)
   _writeoptions(fp,color,None,size,None,width)
   if not datafile:
      n1 = len(xcol)
      n2 = len(ycol)
      n3 = len(anglecol)
      n4 = len(lengthcol)
      if n1 != n2 or n1 != n3 or n1 != n4:
         _error('In vector, xcol, ycol, anglecol, and lengthcol must all have the same number of elements!')
      datafile = tempfile.mktemp()
      _tmplist.append(datafile)
      fp2 = open(datafile,'w')
      for i in range(len(xcol)):
         fp2.write('%4.4e  %4.4e  %4.4e  %4.4e\n' %(xcol[i],ycol[i],anglecol[i],
            lengthcol[i]))
      fp2.close()
      xcol      = 1
      ycol      = 2
      anglecol  = 3
      lengthcol = 4

   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   fp.write('ecol %d\n' %anglecol)
   fp.write('pcol %d\n' %lengthcol)
   _writelimits(fp)
   if pan['logx']:
      fp.write('log x\n')
   if pan['logy']:
      fp.write('log y\n')
   fp.write('vector\n')
   _resetdefaults(fp,color,None,size,None,width)
   _updatepanel(limits,logx,logy,overlap=True)
   fp.close()

def viewport(xmin,xmax=None,ymin=None,ymax=None):
   '''Use this to set the plot area.

      Sometimes WIP gets this wrong and part of your plot is off the page.
      winadj can alter these values, but only within the bounds of the viewport.

      xmin - either a number or a list/tuple of four numbers
      xmax - leave as None if xmin is a list/tuple
      ymin - leave as None if xmin is a list/tuple
      ymax - leave as None if xmin is a list/tuple'''
   fp = _wipopen()
   if _isseq(xmin):
      if len(xmin) != 4:
         _error('You must specify xmin,xmax,ymin,ymax limits for viewport!')
      limits = tuple(xmin)
   else:
      try:
         limits = (float(xmin),float(xmax),float(ymin),float(ymax))
      except ValueError:
         _error('You must specify xmin,xmax,ymin,ymax limits for viewport!')
   fp.write('viewport %f %f %f %f\n' %tuple(limits))
   fp.close()

def wedge(side,offset,thickness,minmax=None,boxarg=None,color=None,font=None,
   size=None,style=None,width=None):
   '''Draw a halftone wedge on the image.

      This is only useful for halftone images.

      side      - string containing side to draw wedge on.  One of left, right,
                  top, or bottom
      offset    - offset of wedge from side
      thickness - thickness of wedge
      minmax    - a list/tuple of the min/max values for the wedge.  Defaults
                  to whatever min/max was for halftone
      boxarg    - argument for box style.  Same as used by axis function
      color     - color for the box around the wedge
      font      - font for labelling the wedge
      size      - size of box labels around wedge
      style     - line style for box
      width     - thickness of box aroun wedge'''
   if side == 'bottom':
      swip = 'b'
   elif side == 'top':
      swip = 't'
   elif side == 'left':
      swip = 'l'
   elif side == 'right':
      swip = 'r'
   else:
      _error('Invalid side parameter.  Try bottom, top, left, or right!')
   fp = _wipopen()
   _writeoptions(fp,color,font,size,style,width)
   fp.write('wedge %s %f %f ' %(swip,offset,thickness))
   if minmax and boxarg:
      fp.write('%f %f %s\n' %(minmax[0],minmax[1],boxarg))
   elif minmax:
      fp.write('%f %f\n' %(minmax[0],minmax[1]))
   elif boxarg:
      fp.write(r'\5 \6 %s' %boxarg)
      fp.write('\n')
   else:
      fp.write('\n')
   _resetdefaults(fp,color,font,size,style,width)
   fp.close()

def winadj(xmin=None,xmax=None,ymin=None,ymax=None,image=None):
   '''Adjust the window so the plot will have the correct aspect ratio.

      xmin   - If set, either a number of a list/tuple of four numbers
      xmax   - leave as None if xmin is a list/tuple
      ymin   - leave as None if xmin is a list/tuple
      ymax   - leave as None if xmin is a list/tuple
      image  - If a string of an image name (and optionally a subimage defined
               ala IRAF's method : image[xmin:xmax,ymin:ymax], then adjust
               window according to image size.  If limits are defined via
               xmin,xmax,ymin,ymax, then multiply nx and ny pixels in the 
               image by the limits to get the true adjustment'''
   global _panellist
   if xmin or xmin == 0:
      if _isseq(xmin):
         if len(xmin) != 4:
            _error('You must specify xmin,xmax,ymin,ymax limits for winadj!')
         limits = tuple(xmin)
      else:
         try:
            limits = (float(xmin),float(xmax),float(ymin),float(ymax))
         except ValueError:
            _error('You must specify xmin,xmax,ymin,ymax limits for winadj!')
   else:
      limits = None
   fp = _wipopen()
   if image:
      _panellist[_curpanel]['image'] = image
      _panellist[_curpanel]['limits'] = None
      blah = _readimage(fp,image)
      if blah:
         if limits:
            nx = limits[1]*(blah[1] - blah[0] + 1)
            ny = limits[3]*(blah[3] - blah[2] + 1)
            fp.write('set \\7 %d\n' %nx)
            fp.write('set \\8 %d\n' %ny)
         else:
            fp.write('set \\7 %f\n' %(blah[1] - blah[0] + 1))
            fp.write('set \\8 %f\n' %(blah[3] - blah[2] + 1))
      elif limits:
         fp.write('set \\7 %f * nx\n' %limits[1])
         fp.write('set \\8 %f * ny\n' %limits[3])
      else:
         fp.write('set \\7 nx\n')
         fp.write('set \\8 ny\n')
      fp.write('winadj 0 \\7 0 \\8\n')
   elif limits:
      fp.write('winadj %f %f %f %f\n' %tuple(limits))
   else:
      fp.write('winadj\n')
   fp.close()

def xlabel(text,offset=0,align='center',color=None,font=None,size=None,
   style=None,width=None,bg=None):
   '''Set the x label to the given text.

      text   - a string of text to print out as x axis label
      offset - offset for text in addition to standard offset for an x label
      align  - alignment for label.  Either left, center, or right
      color  - a string giving the color for the label
      font   - a string giving the font to use for the label
      size   - a number giving the size for label.
      style  - a string giving the line style for the label
      width  - a number giving the width of the lines
      bg     - background color for text'''
   fp = _wipopen()
   al = _translatealign(align)
   off = str(3.2 + float(offset))
   _resetdefaults(fp,1,1,1,1,1,1) # xlabel doesn't properly pick-up default
                                  # parameters that are set so we force them
                                  # to be written out
   # Now override defaults
   _writeoptions(fp,color,font,size,style,width,bg)
   if _isstr(text):
      fp.write('mtext B %s 0.5 %s %s\n' %(off,al,_translatelatex(text)))
   else:
      _error('You must specify a string as the input to xlabel!')
   _resetdefaults(fp,color,font,size,style,width,bg)
   fp.close()

def ylabel(text,offset=0,align='center',color=None,font=None,size=None,
   style=None,width=None,bg=None):
   '''Set the y label to the given text.

      text   - a string of text to print out as y axis label
      offset - offset for text in addition to standard offset for a y label
      align  - alignment for label.  Either left, center, or right
      color  - a string giving the color for the label
      font   - a string giving the font to use for the label
      size   - a number giving the size for label.
      style  - a string giving the line style for the label
      width  - a number giving the width of the lines
      bg     - background color for text'''
   fp = _wipopen()
   if not _isstr(text):
      _error('You must specify a string as the input to ylabel!')
   al = _translatealign(align)
   off = str(2.2 + float(offset))
   _resetdefaults(fp,1,1,1,1,1,1) # ylabel doesn't properly pick-up default
                                  # parameters that are set so we force them
                                  # to all be written out
   # Now override defaults
   _writeoptions(fp,color,font,size,style,width,bg)
   fp.write('mtext L %s 0.5 %s %s\n' %(off,al,_translatelatex(text)))
   _resetdefaults(fp,color,font,size,style,width,bg)
   fp.close()

### Past this point are functions the user should never access directly ###

def _count(datafile):
   '''Count number of non-comment lines in given datafile and return'''
   if os.path.exists(datafile):
      fp = open(datafile,'r')
      line = fp.readline()
      num = 0
      while line:
         if line[0] != '#':
            num = num + 1
         line = fp.readline()
      fp.close()
      return num
   else:
      _error("Datafile %s does not exist!" %datafile)
      return 0

def _error(msg):
   '''Print the error message to standard error'''
   if msg[-1] == '\n':
      sys.stderr.write('### PyWip Error! %s' %msg)
   else:
      sys.stderr.write('### PyWip Error! %s\n' %msg)
   sys.exit()

def _isseq(var):
   '''Test whether var is a list or tuple'''
   ltest = [1,2] # test array for checking whether limits is of type list
   ttest = (1,2) # test array for checking whether limits is of type tuple

   if type(var) in (type(ltest),type(ttest)):
      return 1
   else:
      return 0

def _isstr(var):
   '''Test whether var is a string'''

   s = 'asf' # test string for checking type of var
   if type(s) == type(var):
      return 1
   else:
      return 0

def _maketempfile(xdata,ydata,xerr=None,yerr=None):
   '''Make a temporary data file for reading by wip'''
   global _tmplist

   pan = _updatepanel(None,None,None)
   logx = pan['logx']
   logy = pan['logy']

   n1 = len(xdata)
   n2 = len(ydata)
   if n1 != n2:
      _error('x and y arrays must be the same length!')
   if xerr:
      if len(xerr) != n1:
         _error('xerr array must have same length as x and y arrays!')
   if yerr:
      if len(yerr) != n1:
         _error('yerr array must have same length as x and y arrays!')

   #if n1 == 0:
   #   _error('Zero data points to plot!')
   blah = tempfile.mktemp()
   _tmplist.append(blah)
   fp = open(blah,'w')
   for i in range(len(xdata)):
      fp.write('%6.6e %6.6e ' %(_translatecoords(xdata[i],'ra'),_translatecoords(ydata[i],'dec')))
      if xerr:
         if logx:
            a = float(xdata[i])
            b = float(xerr[i])
            if b == 0:
               fp.write('1 ')
            else:
               fp.write('%6.6e ' %((a+b)/a))
            if a == b:
               fp.write('1 ')
            else:
               # when a-b < 0, this causes the errorbars to be drawn funny
               # due to taking the log of a negative number.  So, we fix
               # by forcing b, the errorbar value to be ~99% of a.  This
               # reduces the a/(a-b) to 99.
               if a -b < 0:
                  fp.write('%6.6e ' %99)
               else:
                  fp.write('%6.6e ' %(a/(a-b)))
         else:
            fp.write('%s ' %xerr[i])
      if yerr:
         if logy:
            a = float(ydata[i])
            b = float(yerr[i])
            if b == 0:
               fp.write('1 ')
            else:
               fp.write('%6.6e ' %((a+b)/a))
            if a == b:
               fp.write('1 ')
            else:
               # when a-b < 0, this causes the errorbars to be drawn funny
               # due to taking the log of a negative number.  So, we fix
               # by forcing b, the errorbar value to be ~99% of a.  This
               # reduces the a/(a-b) to 99.
               if a -b < 0:
                  fp2.write('%6.6e ' %99)
               else:
                  fp.write('%6.6e ' %(a/(a-b)))
         else:
            fp.write('%s ' %yerr[i])
      fp.write('\n')
   fp.close()
   return blah

def _maketempfile2(xcol,ycol,datafile,xerr=None,yerr=None):
   '''Make a temporary data file for reading by wip.  This version reads
      the data from an input file and processes it to an new output file.
      Important for plotting logarithmic errorbars.'''
   global _tmplist


   logx = _panellist[_curpanel]['logx']
   logy = _panellist[_curpanel]['logy']
   blah = tempfile.mktemp()
   _tmplist.append(blah)
   fp1 = open(datafile,'r')
   fp2 = open(blah,'w')
   line = fp1.readline()
   numwritten = 0
   while line:
      if line[0] != '#':
         tmp = line.split()
         fp2.write('%s %s ' %(tmp[xcol - 1],tmp[ycol - 1]))
         if xerr:
            if logx:
               a = float(tmp[xcol - 1])
               b = float(tmp[xerr - 1])
               if a == 0:
                  fp2.write('1 ')
               else:
                  fp2.write('%6.6e ' %((a+b)/a))
               if a == b:
                  fp2.write('1 ')
               else:
                  # when a-b < 0, this causes the errorbars to be drawn funny
                  # due to taking the log of a negative number.  So, we fix
                  # by forcing b, the errorbar value to be ~99% of a.  This
                  # reduces the a/(a-b) to 99.
                  if a -b < 0:
                     fp2.write('%6.6e ' %99)
                  else:
                     fp2.write('%6.6e ' %(a/(a-b)))
            else:
               fp2.write('%s ' %tmp[xerr - 1])
         if yerr:
            if logy:
               a = float(tmp[ycol - 1])
               b = float(tmp[yerr - 1])
               if a == 0:
                  fp2.write('1 ')
               else:
                  fp2.write('%6.6e ' %((a+b)/a))
               if a == b:
                  fp2.write('1 ')
               else:
                  # when a-b < 0, this causes the errorbars to be drawn funny
                  # due to taking the log of a negative number.  So, we fix
                  # by forcing b, the errorbar value to be ~99% of a.  This
                  # reduces the a/(a-b) to 99.
                  if a -b < 0:
                     fp2.write('%6.6e ' %99)
                  else:
                     fp2.write('%6.6e ' %(a/(a-b)))
            else:
               fp2.write('%s' %tmp[yerr - 1])
         fp2.write('\n')
         numwritten = numwritten + 1
      line = fp1.readline()
   fp1.close()
   fp2.close()
   if numwritten == 0:
      _error('Zero data points to plot!')
   return blah

def _plotpoints(fp,xlist,ylist,color=None,size=None,style='o',width=None):
   '''Plot data points from input lists of coordinates.

      This function is a helper to the plot command.  When there are less
      than 10 data points, plot will call this function rather than go through
      the process of making a temp file.  Like plot(), you can show points
      and lines.

      fp        - The file pointer where wip commands are written
      xlist,ylist - Lists or tuples with the x and y positions of points to
                  plot
      color     - If a string, use as the color for every point.  If an integer,
                  read that column from the datafile for color index for each
                  point.
      size      - The size for each data point.
      style     - If a string, use as the symbol or line style.  If an integer,
                  then read from datafile for symbol for each point.
      width     - Line width'''

   pan = _panellist[_curpanel]
   _writeoptions(fp,color,None,size,None,width)
   if pan['logx']:
      for i in range(len(xlist)):
         try:
            xlist[i] = math.log10(xlist[i])
         except ValueError:
            _error("problem with taking log of %f" %xlist[i])
   if pan['logy']:
      for i in range(len(ylist)):
         try:
            ylist[i] = math.log10(ylist[i])
         except ValueError:
            _error("problem with taking log of %f" %ylist[i])
   _writelimits(fp)
   sym = _translatesymbol(style)
   if sym == '99':
      line = _translatelstyle(style)
      fp.write('lstyle %s\n' %line)
      fp.write('move %f %f\n' %(_translatecoords(xlist[0],'ra'),_translatecoords(ylist[0],'dec')))
      for i in range(1,len(xlist)):
         fp.write('draw %f %f\n' %(_translatecoords(xlist[i],'ra'),_translatecoords(ylist[i],'dec')))
      _resetdefaults(fp,color,None,size,style,width)
   else:
      fp.write('symbol %s\n' %sym)
      for a,b in zip(xlist,ylist):
         fp.write('move %f %f\n' %(_translatecoords(a,'ra'),_translatecoords(b,'dec')))
         fp.write('dot\n')
      _resetdefaults(fp,color,None,size,None,width)

def _readimage(fp,image):
   '''Perform image and subimage commands on a given image name.  Return
      x and y pixel limits'''
   idx = image.find('[')
   if idx != -1:
      blah = image[idx+1:-1] # x and y range
      blah = blah.split(',')
      if len(blah) != 2:
         _error('You must specify image range as [xmin:xmax,ymin:ymax]!')
      t1   = blah[0].split(':')
      t2   = blah[1].split(':')
      if len(t1) != 2 or len(t2) != 2:
         _error('You must specify image range as [xmin:xmax,ymin:ymax]!')
      try:
         xmin = int(t1[0])
         xmax = int(t1[1])
         ymin = int(t2[0])
         ymax = int(t2[1])
      except ValueError:
         _error('Image range must be integer pixel values!')
      if os.path.exists(image[:idx]):
         fp.write('image %s\n' %image[:idx])
      else:
         _error('Image %s does not exist!' %image[:idx])
      fp.write('subimage %d %d %d %d\n' %(xmin,xmax,ymin,ymax))
      return (xmin,xmax,ymin,ymax)
   else:
      if os.path.exists(image):
         fp.write('image %s\n' %image)
      else:
         _error('Image %s does not exist!' %image)
      return None

def _resetdefaults(fp,color=None,font=None,size=None,style=None,width=None,
   bg=None):
   '''Resets any parameter changes to color, font, size, line style, or line
      width that may have been made.
      
      This is called when you change the value of size within a single function
      but don't want to propigate this change to other functions. To change
      default values permanently within a plot, see the default() function. To
      reset the defaults to their true defaults (defined at the top of file),
      use the _setglobaldefaults() function.'''
      
   if color:
      fp.write('color %s\n' %_color)
   if font:
      fp.write('font %s\n' %_font)
   if size:
      fp.write('expand %s\n' %_size)
   if style:
      fp.write('lstyle %s\n' %_lstyle)
   if width:
      fp.write('lwidth %s\n' %_lwidth)
   if bg:
      fp.write('bgci %s\n' %_bg)

def _setglobaldefaults():
   '''Default values for font, color, size, line width, line style, and 
      background color.
      
      These values will be set anytime savefig() or _wipopen() is called.  This
      means if you have multiple plots in one script, the defaults will be
      reset before starting the new plot.  It's a little redundant to call this
      function with both savefig() and _wipopen(), but better safe than sorry.'''

   global _wipfile, _tmplist, _panellist, _curpanel, _font, _lwidth, _lstyle
   global _color, _size, _bg
   
   _wipfile   = '???' # the temporary wip file that holds all the plot commands
   _tmplist   = []    # list of any temp files we have made
   _panellist = []    # list of dictionaries containing info about limits and
                      # logx/y for each panel
   _curpanel  = 0     # index of current panel number in _panellist
   _font      = '2'   # default font (roman)
   _lwidth    = '1'   # default line width
   _lstyle    = '1'   # default line style (solid)
   _color     = '1'   # default color (black)
   _size      = '1'   # default size
   _bg        = '-1'  # default background text color, i.e. transparent

def _translatealign(align):
   '''take useful alignment string and convert to wip format'''
   if align == 'left':
      return '0.0'
   elif align == 'center':
      return '0.5'
   elif align == 'right':
      return '1.0'
   else:
      _error('Invalid alignment.  Try left, center, or right.')

def _translatecolor(col):
   '''Take useful color string and convert to wip format.  Note that for
      k and w, I assume you have changed your PGPLOT_BACKGROUND and
      PGPLOT_FOREGROUND colors so that black and white are switched.'''

   colorstr = str(col)
   for i in range(len(_colors)):
      if colorstr == _colors[i]:
         return str(i)
   _error('Invalid color name %s' %colorstr)

def _translatecoords(text,coord):
   '''Translate ra/dec coordinates into ones useful for WIP'''
   s = 'asfa' # temp string to test for type of input argument
   if type(text) == type(s): # if a string, assume we have ra/dec coords
      tmp = text.split(':')
      mul = 3600.0
      outval = 0
      for x in tmp:
         outval = outval + abs(mul*float(x))
         mul = mul/60.0
      if float(tmp[0]) < 0:
         outval = -1*outval
      if len(tmp) == 1: # didn't split by :, so assume user input degrees
         if coord == 'ra':
            outval = outval/15.0 # convert arcseconds to hour-seconds
      return outval
   else: # If user didn't give a string, assume coordinates are okay as-is
      return text
   
def _translatefill(fill):
   '''take useful fill string and convert to wip format.  This is the type of
      fill string used for boxes.  For filled symbols, see below.'''
   fillstr = str(fill)
   for i in range(len(_fills)):
      if fillstr == _fills[i]:
         return str(i+1)
   _error('Invalid fill style %s.  Try s,h,/, or #.' %fillstr)

def _translatefillsymbol(style):
   '''Translate a symbol style into a fill style (later retranslated by 
      _translatesymbol.  This is for filled symbols.  For filling of boxes, see
      above'''
   if style:
      if   style == 'o':  fillstyle = 'fo'
      elif style == '^':  fillstyle = 'f^'
      elif style == 's':  fillstyle = 'fs'
      elif style == 'st': fillstyle = 'fst'
      else: _error('Only circles, triangles, squares, and five-point stars can have a fill color!')
   else:
      if   _style == '4' : fillstyle = 'fo'
      elif _style == '7' : fillstyle = 'f^'
      elif _style == '0' : fillstyle = 'fs'
      elif _style == '12': fillstyle = 'fst'
      else: _error('Only circles, triangles, squares, and five-point stars can have a fill color!')

   if fillstyle in ['fo','fs']:
      fillfactor = 1.3
   elif fillstyle == 'fst':
      fillfactor = 0.8
   else:
      fillfactor = 0.8
   
   return fillstyle,fillfactor

def _translatefont(font):
   '''Translate a useful font name into wip'''
   fontstr = str(font)
   for i in range(len(_fonts)):
      if fontstr == _fonts[i]:
         return str(i+1)
   _error('Invalid font %s.  Try rm, it, sf, or cu!' %fontstr)

def _translatelatex(latex):
   '''Translate latex string into something usable by WIP'''

   greeklatex = (r'\alpha',r'\beta',r'\xi',r'\delta',r'\epsilon',r'\phi',
      r'\gamma',r'\theta',r'\iota',r'\kappa',r'\lambda',r'\mu',r'\nu',r'\pi',
      r'\psi',r'\rho',r'\sigma',r'\tau',r'\upsilon',r'\omega',r'\chi',r'\eta',
      r'\zeta',r'\Xi',r'\Delta',r'\Phi',r'\Gamma',r'\Theta',r'\Lambda',r'\Pi',
      r'\Psi',r'\Sigma',r'\Upsilon',r'\Omega')
   wiplatex = (r'\ga',r'\gb',r'\gc',r'\gd',r'\ge',r'\gf',r'\gg',r'\gh',r'\gi',
      r'\gk',r'\gl',r'\gm',r'\gn',r'\gp',r'\gq',r'\gr',r'\gs',r'\gt',r'\gu',
      r'\gw',r'\gx',r'\gy',r'\gz',r'\gC',r'\gD',r'\gF',r'\gG',r'\gH',r'\gL',
      r'\gP',r'\gQ',r'\gS',r'\gU',r'\gW')
   stack = [] # keep track of super/subscript stuff

   if _font   == '1':
      defaultfont = r'\fn'
   elif _font == '2':
      defaultfont = r'\fr'
   elif _font == '3':
      defaultfont = r'\fi'
   elif _font == '4':
      defaultfont = r'\fs'
   else:
      _error('Invalid default font: %s!' %defaultfont)

   outstr = latex
   #outstr = r'\fi' + outstr # default to italics font for latex mode
   for g,w in zip(greeklatex,wiplatex):
      outstr = outstr.replace(g,w)
   i = 0
   outstr = outstr.replace(r'\times',r'\x')
   outstr = outstr.replace(r'\AA','\A')
   outstr = outstr.replace(r'\odot',r'\(2281)')
   outstr = outstr.replace(r'\oplus',r'\(2284)')
   outstr = outstr.replace(r'\pm',r'\(2233)')
   outstr = outstr.replace(r'\geq',r'\(2244)')
   outstr = outstr.replace(r'\leq',r'\(2243)')
   while i < len(outstr):
      if outstr[i:i+2] == '^{':
         outstr = outstr[:i] + r'\u' + outstr[i+2:]
         i = i + 2
         stack.append(r'\d')
      elif outstr[i:i+2] == '_{':
         outstr = outstr[:i] + r'\d' + outstr[i+2:]
         i = i + 2
         stack.append(r'\u')
      elif outstr[i:i+4] == r'\sf{':
         outstr = outstr[:i] + r'\fn' + outstr[i+4:]
         i = i + 4
         stack.append(defaultfont)
      elif outstr[i:i+4] == r'\rm{':
         outstr = outstr[:i] + r'\fr' + outstr[i+4:]
         i = i + 4
         stack.append(defaultfont)
      elif outstr[i:i+4] == r'\it{':
         outstr = outstr[:i] + r'\fi' + outstr[i+4:]
         i = i + 4
         stack.append(defaultfont)
      elif outstr[i:i+4] == r'\cu{':
         outstr = outstr[:i] + r'\fs' + outstr[i+4:]
         i = i + 4
         stack.append(defaultfont)
      elif outstr[i:i+2] == r'\{':
         outstr = outstr[:i] + '{' + outstr[i+2:]
         i = i + 2
      elif outstr[i:i+2] == '\}':
         outstr = outstr[:i] + '}' + outstr[i+2:]
         i = i + 2
      elif outstr[i] == '}':
         try:
            char = stack.pop()
            outstr = outstr[:i] + char + outstr[i+1:]
         except IndexError: # emptystack
            pass
         i = i + 1
      else:
         i = i + 1
   # fix bug where the carat, ^, doesn't render properly with WIP
   outstr = outstr.replace(r'^',r'\(756)')
   return outstr

def _translatelevels(levels):
   '''Translate levels which can be a list, tuple, or string into something 
      usable by wip'''
      
   if _isstr(levels):
      levs = []
      blah  = levels.split(',')
      blah2 = levels.split(':')
      if len(blah) > 1:
         for b in blah:
            levs.append(float(b))
      elif len(blah2) > 1:
         try:
            xstart = float(blah2[0])
            xstop  = float(blah2[1])
            step   = float(blah2[2])
            levs = []
            x = xstart
            if xstop > xstart and step > 0:
               while x <= xstop:
                  levs.append(x)
                  x = x + step
            elif xstop < xstart and step < 0:
               while x >= xstop:
                  levs.append(x)
                  x = x + step
            else:
               _error('Levels %s is a an infinite loop!' %levels)
         except IndexError:
            _error('Levels range must be specified as xstart:xstop:step!')
      else:
         try:
            levs.append(float(levels))
         except ValueError:
            _error('You must give at least one number for levels!')
      return string.join(str(levs)[1:-1].split(',')) # [1:-1] to strip off []
   elif _isseq(levels):
      return string.join(str(levels)[1:-1].split(','))
   else:
      _error('You must give a list/tuple or a string for the levels command!')

def _translatepalette(palette):
   '''Translate a useful palette string to wip format'''

   palettestr = str(palette)
   negFlag = 0 # set to one if we want the reverse palette
   if palettestr[0] == '-':
      negFlag = 1
      palettestr = palettestr[1:]
   for i in range(len(_palettes)):
      if palettestr == _palettes[i]:
         if negFlag:
            return str(-1*i)
         else:
            return str(i)
   _error('Invalid palette %s!' %palettestr)

def _translatelstyle(lstyle):
   '''Translate a useful line style into wip'''
   lstylestr = str(lstyle)
   for i in range(len(_lstyles)):
      if lstylestr == _lstyles[i]:
         return str(i+1)
   _error('Invalid line style %s.  Try - , -- , .- , : , or -...' %lstylestr)

def _translatesymbol(sym):
   '''take useful symbol string and convert to wip format'''
   symbol = str(sym) # ensure we have a string
   if symbol == 's': # square
      return '0'
   elif symbol == '.': # dot
      return '1'
   elif symbol == '+': # plus sign
      return '2'
   elif symbol == '*': # asterisks
      return '3'
   elif symbol == 'o': # circle
      return '4'
   elif symbol == 'x': # cross
      return '5'
   elif symbol == '^': # triangle
      return '7'
   elif symbol == 'oplus': # circle with plus sign
      return '8'
   elif symbol == 'odot': # circle with dot
      return '9'
   elif symbol == 'ps': # pointed square
      return '10'
   elif symbol == 'd': # diamond
      return '11'
   elif symbol == 'st': # five-point star
      return '12'
   elif symbol == 'f^': # filled triangle
      return '13'
   elif symbol == 'o+': # open plus symbol
      return '14'
   elif symbol == 'david': # star of david
      return '15'
   elif symbol == 'fs': # filled square
      return '16'
   elif symbol == 'fo': # filled circle
      return '17'
   elif symbol == 'fst': # filled five-point star
      return '18'
   else:
      return '99'

def _updatepanel(limits,logx,logy,image=None,overlap=None):
   '''Update info on limits and whether logx or logy is being plotted'''
   global _panellist

   if limits:
      if _isseq(limits):
         if len(limits) == 4:
            _panellist[_curpanel]['limits'] = limits
         else:
            _error('You must specify four limits as xmin,xmax,ymin,ymax!')
      else:
         idx = _curpanel - 1
         while idx >= 0:
            if _isseq(_panellist[idx]['limits']):
               _panellist[_curpanel]['limits'] = _panellist[idx]['limits']
               break
            idx = idx - 1
   else:
      if not _isseq(_panellist[_curpanel]['limits']):
         _panellist[_curpanel]['limits'] = 1
   if logx:
      _panellist[_curpanel]['logx'] = logx
   if logy:
      _panellist[_curpanel]['logy'] = logy
   if image:
      _panellist[_curpanel]['image'] = image
   if overlap:
      _panellist[_curpanel]['overlap'] = overlap
   return _panellist[_curpanel]

def _vptoxy(fp,x,y,r1,r2):
   '''Convert viewport x/y to physical x/y
      x/y   - floats of x/y viewport values
      r1,r2 - strings of register names to set holding values'''
   fp.write(r'set %s ((x2 - x1) * (%s - vx1) / (vx2 - vx1)) + x1' %(r1,x))
   fp.write('\n')
   fp.write(r'set %s ((y2 - y1) * (%s - vy1) / (vy2 - vy1)) + y1' %(r2,y))
   fp.write('\n')

def _wipopen():
   '''Open the wip file for writing.  If one does not already exist, start a
      new one'''

   global _wipfile,_panellist
   
   if _wipfile == '???':
      _setglobaldefaults() # set all parameters to their default values
      tempfile.tempdir = os.getcwd()
      _wipfile = tempfile.mktemp(suffix='.wip')
      fp = open(_wipfile,'w')
      fp.write('set print ignore\n')
      fp.write('color %s\n'  %_color)
      fp.write('font %s\n'   %_font)
      fp.write('expand %s\n' %_size)
      fp.write('lstyle %s\n' %_lstyle)
      fp.write('lwidth %s\n' %_lwidth)
      fp.write('bgci %s\n'   %_bg)
      _panellist = [{'logx' : False, 'logy' : False, 'limits' : None,
      'image' : None, 'overlap' : False, 'nx' : 1, 'ny' : 1}]
   else:
      fp = open(_wipfile,'a')
   return fp

def _writeimglimits(fp,limits):
   '''Write limits for an image.  If limits = None, do nothing.  We will plot
      the entire image.  If limits is a list or tuple of four numbers, then
      set limits to given values.  Otherwise, set limits to register variables
      \1 \2 \3 \4.'''
   global _panellist

   if limits:
      if _isseq(limits):
         if len(limits) != 4:
            _error('You must specify four limits: [xmin,xmax,ymin,ymax]!')
         fp.write('limits %f %f %f %f\n' %tuple(limits))
         fp.write('set \\1 x1\n') # store map limits in registers 1-4
         fp.write('set \\2 x2\n') # useful for contour overlays
         fp.write('set \\3 y1\n')
         fp.write('set \\4 y2\n')
         _panellist[_curpanel]['limits'] = 1
      else:
         fp.write('limits \\1 \\2 \\3 \\4 \n')
         _panellist[_curpanel]['limits'] = 1
   else:
      if _panellist[_curpanel]['limits']:
         fp.write('limits \\1 \\2 \\3 \\4 \n')

def _writelimits(fp):
   '''Write limits for a data file.  If limits = None, then set limits using
      the limits command.  Other possibilities are to set the limits to the
      specified values, or don't touch them at all (useful for overplotting)'''

   limits = _panellist[_curpanel]['limits']
   logx   = _panellist[_curpanel]['logx']
   logy   = _panellist[_curpanel]['logy']
   overlap= _panellist[_curpanel]['overlap']
   if not limits:
      fp.write('limits\n')
   else:
      if _isseq(limits):
         if len(limits) != 4:
            _error('You must specify four limits: [xmin,xmax,ymin,ymax]')
         fp.write('limits ')
         if logx:
            fp.write('%f %f ' %(math.log10(limits[0]),math.log10(limits[1])))
         else:
            fp.write('%f %f ' %(limits[0],limits[1]))
         if logy:
            fp.write('%f %f ' %(math.log10(limits[2]),math.log10(limits[3])))
         else:
            fp.write('%f %f ' %(limits[2],limits[3]))
         fp.write('\n')
      elif not overlap:
         fp.write('limits\n')

def _writeoptions(fp,color=None,font=None,size=None,style=None,width=None,
   bg=None):
   '''Writes out any changes to color, font, size, line style, or line
      width that may have been made.'''
   if color:
      col = _translatecolor(color)
      fp.write('color %s\n' %col)
   if font:
      fn  = _translatefont(font)
      fp.write('font %s\n' %fn)
   if size:
      fp.write('expand %s\n' %size)
   if style:
      sty = _translatelstyle(style)
      fp.write('lstyle %s\n' %sty)
   if width:
      fp.write('lwidth %s\n' %width)
   if bg:
      fp.write('bgci %s\n' %_translatecolor(bg))

def _xytovp(fp,x,y,r1,r2):
   '''Convert x/y values to viewport values
      x,y   - x and y coordinates as floats
      r1,r2 - strings of registers to set holding values'''
   fp.write(r'set %s ((vx2 - vx1) * (%s - x1) / (x2 - x1)) + vx1' %(r1,x))
   fp.write('\n')
   fp.write(r'set %s ((vy2 - vy1) * (%s - y1) / (y2 - y1)) + vy1' %(r2,y))
   fp.write('\n')
