#!/usr/bin/env python
r'''Library functions for running WIP within python
Started 12 April 2007 - NLC
First used for a real plot on 19 April 2007
version 2.0 - 21 April 2009 - Lots of backend changes, switched to variable
   arguments in all function calls, made the uber-awesome legend command,
   and made the axis command user-friendly.  It is NOT backwards-compatible
   with the older version1, but it is easy to update your scripts.  Only a
   few of the commands have changed argument calls: axis, curve, legend, panel,
   wedge, and winadj.

last updated: 20 May 2010 - NLC

       Colors                          Palettes
=========================     ===============================
| key | Description     |     |   key   |    Description    |
-------------------------     -------------------------------
|  w  | white           |     |  gray   | grayscale         |
|  k  | black (default) |     | rainbow | Rainbow scale     |
|  r  | red             |     |  heat   | Heat scale        |
|  g  | green           |     |  iraf   | IRAF scale        |
|  b  | blue            |     |  aips   | AIPS scale        |
|  c  | cyan            |     | pgplot  | PGPLOT scale      |
|  m  | magenta         |     |    a    | SAOimage A scale  |
|  y  | yellow          |     |   bb    | SAOimage BB scale |
|  o  | orange          |     |   he    | SAOimage HE scale |
| gy  | green-yellow    |     |   i8    | SAOimage I8 scale |
| gc  | green-cyan      |     |   ds    | DS scale          |
| bc  | blue-cyan       |     | cyclic  | Cyclic scale      |
| bm  | blue-magenta    |     -------------------------------
| rm  | red-magenta     |     * can prepend palette keywords with
| dg  | dark gray       |       a minus sign to reverse the scale
| lg  | light gray      |
-------------------------

    Fill Styles                     Fonts
=======================   =========================
| key |  Description  |   | key |   Description   |
-----------------------   -------------------------
|  s  | solid         |   |  sf | sans-serif      |
|  h  | hollow        |   |  rm | roman (default) |
|  /  | hatched       |   |  it | italics         |
|  #  | cross-hatched |   |  cu | cursive         |
-----------------------   -------------------------

             Symbols                               Line Styles
=================================        ===========================
|  key  |       Description     |        | key  |    Description   |
---------------------------------        ---------------------------
|   s   | square                |        |   -  | solid (default)  |
|   .   | dot                   |        |  --  | dashed           |
|   +   | plus sign             |        |  .-  | dot-dashed       |
|   *   | asterisks             |        |   :  | dotted           |
|   o   | circle (default)      |        | -... | dash-dot-dot-dot |
|   x   | an x                  |        ---------------------------
|   ^   | triangle              |
| oplus | circle with plus sign |
| odot  | circle with a dot     |
|  ps   | pointed square        |
|   d   | diamond               |
|  st   | five-point star       |
|  o+   | open plus symbol      |
| david | star of david         |
---------------------------------
* Only the square, circle, triangle, and five-point star can be filled.

LaTeX is supported in text strings by default.  Note that unlike math mode in
LaTeX, the text is NOT italicized by  default.  LaTeX symbols supported: all
greek symbols, \times, \AA (for angstroms), \odot (for sun symbol),
\oplus (for earth symbol), \pm (for proper +-), \geq (for greater than or
equal to), \leq (for less than or equal to), ^{} (for superscripts),
_{} (for subscripts), \sf{} (for sans-serif font), \rm{} (for roman font),
\it{} (for italics), \cu{} (for cursive)'''

import os,sys,tempfile,string,math,re

_palettes = ('gray','rainbow','heat','iraf','aips','pgplot','a','bb','he','i8','ds','cyclic')

_colors = ('w','k','r','g','b','c','m','y','o','gy','gc','bc','bm','rm','dg','lg')

_fills = ('s','h','/','#')

_fonts = ('sf','rm','it','cu')

_lstyles = ('-','--','.-',':','-...')

_symbols = ('s','.','+','*','o','x','^','oplus','odot','ps','d','st','o+','david')


# Private variables set by _wipopen().

_wipfile    = '???' # the temporary wip file that holds all the plot commands
_tmplist    = []    # list of any temp files we have made
_optionsobj = None  # set to class _options by _wipopen()
_panelobj   = None  # set to class _panel by panel()

def arc(x,y,majorx,majory,deg=360,start=0,angle=0,fill='s',**args):
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
      angle         - Tilt angle in degrees measured counter-clockwise
                      from +x axis.
      fill          - a string specifying the fill style
      Allowed optional **args:
         color     - a string with the color of the line
         style     - a string specifying the line style
         width     - thickness of lines
         limits    - If a list/tuple, use as limits.  Otherwise try to use
                     any prexisting limits or set new ones
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''
   fp = _wipopen('arc')
   _panelobj.writelimits(fp,**args)
   _optionsobj.update(fp,**args)
   fp.write('fill %s\n' %_translatefill(fill))
   fp.write('move %f %f\n' %(_translatecoords(x,coord='ra'),_translatecoords(y,coord='dec')))
   fp.write('angle %f\n' %angle)
   tmpx = _translatecoords(majorx,coord='ra')
   tmpy = _translatecoords(majory,coord='dec')
   fp.write('arc %f %f %f %f\n' %(tmpx,tmpy,deg,start))
   _optionsobj.reset(fp,**args)
   fp.write('angle 0\n')
   fp.close()

def arrow(xcol,ycol,taper=45,vent=0.0,fill='s',**args):
   '''Draw a single arrow.

      If you want many arrows, consider using the vector() command.  The
      coordinates given by xcol and ycol can be WCS coordinates, following the
      usual style for specifying them, i.e. as a sexagesimal string or a string
      for the coordinate in degrees.

      xcol   - a list/tuple of the begin and end x coordinates of the arrow
      ycol   - a list/tuple of the begin and end y coordinates of the arrow
      taper  - tapering angle, in degrees, for the arrowhead
      vent   - fraction of the arrowhead that is cut-away in the back.
      fill   - fill style for the arrowhead
      Allowed optional **args:
         color     - the color of the arrow and arrowhead as a string
         size      - the size of the arrowhead
         style     - line style for drawing the arrow
         width     - thickness of the line
         limits    - If a list/tuple, use as limits.  Otherwise try to use
                     any prexisting limits or set new ones
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''

   if not _isseq(xcol) or len(xcol) != 2:
      _error('arrow(): xcol must be a list/tuple of 2 elements!')
   if not _isseq(ycol) or len(ycol) != 2:
      _error('arrow(): ycol must be a list/tuple of 2 elements!')

   fp = _wipopen('arrow')
   _panelobj.writelimits(fp,**args)
   _optionsobj.update(fp,**args)
   tmpx = [_translatecoords(xcol[0],coord='ra') ,_translatecoords(xcol[1],coord='ra')]
   tmpy = [_translatecoords(ycol[0],coord='dec'),_translatecoords(ycol[1],coord='dec')]
   if _panelobj.get('logx'):
      tmpx = [math.log10(tmpx[0]),math.log10(tmpx[1])]
   if _panelobj.get('logy'):
      tmpy = [math.log10(tmpy[0]),math.log10(tmpy[1])]
   fp.write('fill %s\n' %_translatefill(fill))
   fp.write('move %f %f\n'  %(tmpx[0],tmpy[0]))
   fp.write('arrow %f %f %f %f\n' %(tmpx[1],tmpy[1],taper,vent))
   _optionsobj.reset(fp,**args)
   fp.close()

def axis(**args):
   '''Draw the axes.

      There are many allowed **args.  Most of the time you can probably get by
      without specifying any of them.  If you need to tweak the plotting of the
      axis, consult the User's Manual for a table of all the keywords.

      Additionally, there are the "standard" optional **args:
         color - the color of the axis as a string
         font  - the font to use for the axis
         size  - the size for numbers on axis
         style - the line style
         width - the thickness of the lines'''

   nx    = _panelobj.nx # number of panels in x and y directions
   ny    = _panelobj.ny # used for automatically labelling axes
   image = _panelobj.get('image') # name of image in panel

   fp = _wipopen('axis')
   _optionsobj.update(fp,**args)
   if args.has_key('xinterval'):
      xtick = args['xinterval']
   else:
      xtick = (0,0)
   if args.has_key('yinterval'):
      ytick = args['yinterval']
   else:
      ytick = (0,0)
   if _isseq(xtick) and len(xtick) == 2:
      pass
   else:
      _error('axis(): You must provide two arguments for xinterval!')
   if _isseq(ytick) and len(ytick) == 2:
      pass
   else:
      _error('axis(): You must provide two arguments for yinterval!')

   if args.has_key('xinterval') or args.has_key('yinterval'):
      fp.write('ticksize %g %g '  %tuple(xtick))
      fp.write('%g %g\n' %tuple(ytick))

   if not args.has_key('box'):
      args['box'] = ('bottom','left','top','right')
   for k in ('logx','logy'):
      if not args.has_key(k):
         if _panelobj.get(k): args[k] = True
   if not args.has_key('number'):
      args['number'] = []
      # space between panels or on last row
      if _panelobj.gapy > 0 or _panelobj.idx/nx == (ny - 1):
         args['number'].append('bottom')
      # space between panels or on first column
      if _panelobj.gapx > 0 or _panelobj.idx%nx == 0:
         args['number'].append('left')
   if _panelobj.get('image'):
      if not args.has_key('format'):
         hd = _panelobj.get('header')
         if hd == 'rd':
            args['format'] = ['wcs','wcs']
      if not args.has_key('verticaly'):
         args['verticaly'] = True
   if args.has_key('drawtickx') and not args['drawtickx']: # exists and False
      args['subtickx'] = False
      args['majortickx'] = False
   else:
      for k in ('subtickx','majortickx'):
         if not args.has_key(k): args[k] = True
   if args.has_key('drawticky') and not args['drawticky']: # exists and False
      args['subticky'] = False
      args['majorticky'] = False
   else:
      for k in ('subticky','majorticky'):
         if not args.has_key(k): args[k] = True

   xlab,ylab = _translateaxis(**args)
   fp.write('box %s %s\n' %(xlab,ylab))
   _optionsobj.reset(fp,**args)
   if args.has_key('xinterval') or args.has_key('yinterval'): # unset tick values
      fp.write('ticksize 0 0 0 0\n')
   fp.close()

def beam(x,y,amajor,aminor,angle=0,scale='rd',**args):
   '''Draw an ellipse showing a beam.  Useful for radio data.

      x,y           - Central coordinates for beam.  If a string, convert WCS
                      into units usable by WIP.  The WCS can be given as
                      hh:mm:ss, dd:mm:ss or as degrees.
      amajor,aminor - major and minor axes for ellipse as arcseconds. Like x,y
                      this can be given as a string to specify the WCS.
      angle         - tilt angle for ellipse given as degrees from the +x axis
      scale         - set to None for no scaling of x axis
      Allowed optional **args:
         color     - color of edge of ellipse
         fillcolor - color of inside of ellipse.  Defaults to color
         style     - line style for edge of ellipse
         width     - line thickness
         bg        - color for background box surrounding beam.  Defaults to
                     transparent.'''
   fp = _wipopen('beam')
   if scale == 'rd':
      scl = '-1'
   else:
      scl = '1'
   if args.has_key('bg'):
      bgrect = _translatecolor(args['bg'])
   else:
      bgrect = _optionsobj.bg
   if args.has_key('fillcolor'):
      fillcl = _translatecolor(args['fillcolor'])
   elif args.has_key('color'):
      fillcl = _translatecolor(args['color'])
   else:
      fillcl = _optionsobj.color
   _optionsobj.update(fp,**args)
   fp.write('move %f %f\n' %(_translatecoords(x,coord='ra'),_translatecoords(y,coord='dec')))
   tmpx = _translatecoords(amajor,coord='ra')
   tmpy = _translatecoords(aminor,coord='dec')
   fp.write('beam %f %f %f 0 0 %s %s %s\n' %(tmpx,tmpy,angle,scl,fillcl,bgrect))
   _optionsobj.reset(fp,**args)
   fp.close()

def bin(xcol,ycol,datafile=None,coord='center',text=None,**args):
   '''Draw a histogram from previously histogrammed data.

      xcol,ycol - either an integer specifying column in datafile or a
                  list/tuple of x/y data points
      datafile  - string containing filename with data.  Leave as None if
                  xcol and ycol are tuples/lists
      coord     - are x coordinates the center or left side of the bin?
      text      - Text to describe the curve in the legend
      Allowed optional **args:
         color     - the color for the histogram
         style     - line style for histogram
         width     - the thickness of the histogram
         limits    - If a list/tuple, use as limits.  Otherwise try to use
                     any prexisting limits or set new ones
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''
   if not args.has_key('style'):
      args['style'] = '-'
   curve(text=text,**args)
   fp = _wipopen('bin')
   if   coord == 'center': k = 1
   elif coord == 'left':   k = 0
   else:   _error('bin(): coord must be either center or left.')

   if not datafile:
      datafile = _maketempfile(xcol,ycol)
      xcol = 1
      ycol = 2
   _panelobj.set(**args)
   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   _panelobj.writelimits(fp,**args)
   if _panelobj.get('logx'): fp.write('log x\n')
   if _panelobj.get('logy'): fp.write('log y\n')
   _optionsobj.update(fp,**args)
   fp.write('bin %d\n' %k)
   _optionsobj.reset(fp,**args)
   fp.close()

def blowup(xmin,xmax,ymin,ymax,corners=['ll','ul'],**args):
   r'''Draw a blow-up box around a given region.

      Use in conjunction with connect() to draw connecting lines from the
      blowup box to the actual zoomed region.

      xmin,xmax,ymin,ymax - four values for limits of blowup box given as
                            pixel values
      corners - a two element list of ll,ul,lr, or ur to specify which corners
                of the box to store in memory.  These correspond to lower left,
                upper left,lower right, upper right
      Allowed optional **args:
         color - a string giving the color for the box
         style - a string giving the linestyle for the box
         width - a number giving the thickness of the box'''

   fp = _wipopen('blowup')
   _optionsobj.update(fp,**args)
   fp.write('limits 1 \\7 1 \\8\n') # ensures that xmin,xmax,ymin,ymax are all
                                    # pixel coordinates.  Assumes the user has
                                    # set \7 and \8 through winadj
   fp.write('fill %s\n' %_translatefill('h')) # always hollow
   fp.write('rect %f %f %f %f\n' %(xmin,xmax,ymin,ymax))
   if   corners[0] == 'll':  _xytovp(fp,str(xmin),str(ymin),r'\9',r'\10')
   elif corners[0] == 'ul':  _xytovp(fp,str(xmin),str(ymax),r'\9',r'\10')
   elif corners[0] == 'lr':  _xytovp(fp,str(xmax),str(ymin),r'\9',r'\10')
   elif corners[0] == 'ur':  _xytovp(fp,str(xmax),str(ymax),r'\9',r'\10')

   if   corners[1] == 'll':  _xytovp(fp,str(xmin),str(ymin),r'\11',r'\12')
   elif corners[1] == 'ul':  _xytovp(fp,str(xmin),str(ymax),r'\11',r'\12')
   elif corners[1] == 'lr':  _xytovp(fp,str(xmax),str(ymin),r'\11',r'\12')
   elif corners[1] == 'ur':  _xytovp(fp,str(xmax),str(ymax),r'\11',r'\12')
   _optionsobj.reset(fp,**args)
   fp.close()

def connect(corners=['ll','ul'],**args):
   '''Draw lines connecting a blow-up region to the current viewport coordinates.

      corners - a two element list of ll,ul,lr, or ur to specify which corners
                of the box to store in memory.  These correspond to lower left,
                upper left,lower right, upper right
      Allowed optional **args:
         color - a string giving the color for the lines
         style - a string giving the linestyle for the lines
         width - a number giving the thickness of the lines'''

   fp = _wipopen('connect')
   _optionsobj.update(fp,**args)
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
   _optionsobj.update(fp,**args)
   fp.close()

def contour(image,levels,header='rd',routine='smooth',unit='step',**args):
   r'''Draw contours with the specified levels.

      image   - string with name of image
      levels  - a list/tuple,string,or integer of contour levels.
         list/tuple - draw contours at specified levels
         number     - If unit='nbin', autogenerate the given number of
                      levels between the min and max.  unit='step' is not
                      supported (there is no way for wip/pywip to know the min
                      value).
         string     - If 'border', draw a box around the border of the
                      image.  Otherwise, must be three numbers formatted as
                      'val1:val2:val3'.  Depending on the keyword unit, the
                      values have different meanings:
                         unit='step'  - string is 'min:max:step'
                         unit='nbin'  - string is 'min:max:nbin'
                         unit='sigma' - string is 'start:step:sigma', e.g.
                            '3:1:0.1' would start at 3 sigma, in steps of
                            1 sigma, with 1 sigma being 0.1.
      header  - string of header for image.  x and y can be different, e.g.
         'rd px', but if only one value is given, it will be the same for y
         as well.  The allowed values are:
            rd - Right Ascension/Declination
            so - Arcsecond offset
            mo - Arcminute offset
            po - pixel offset
            px - Absolute pixels
            gl - General linear coordinates
            go - General linear coordinate offsets
      routine - how to draw contours: smooth, fast, neg.  neg will draw negative
                contours with the same line style as positive ones.  By default,
                negative contours are drawn dashed.
      unit    - units for levels keyword.  See description of levels.
      Allowed optional **args:
         limits  - The standard limits keyword
         color   - color for contour lines
         font    - font for contour labelling (TODO: not supported)
         style   - line style for contours
         width   - thickness of contour lines'''

   fp = _wipopen('contour')
   if levels == 'border':
      borderFlag = True
   else:
      borderFlag = False

   if _panelobj.get('image') != image:
      _readimage(fp,image)
   _panelobj.set(**args)
   fp.write('header %s\n' %header)
   if borderFlag:
      fp.write('set \\13 x1\n')
      fp.write('set \\14 x2\n')
      fp.write('set \\15 y1\n')
      fp.write('set \\16 y2\n')
   else:
      blah = _translatelevels(levels,unit)
      if isinstance(blah,str):
         fp.write('levels %s\n' %blah)
         if unit == 'sigma':
            tmp = levels.split(':')
            fp.write('slevel a %s\n' %tmp[2])
         else:
            fp.write('slevel a 1\n')
      else: # use autolevs
         if unit == 'nbin':
            fp.write('autolevs %d\n' %blah)
         elif unit == 'step':
            _error('_contour(): unit=step not allowed when level=a number!')
         #if _isseq(minmax) and len(minmax) == 2:
         #   fp.write('autolevs %d lin %g %g\n' %(blah,minmax[0],minmax[1]))
         #else:
         #   fp.write('autolevs %d\n' %blah)
   if _panelobj.get('limits'):
      fp.write('limits \\1 \\2 \\3 \\4 \n')
   else:
      fp.write('set \\1 x1\n') # store map limits in registers 1-4
      fp.write('set \\2 x2\n') # useful for contour overlays
      fp.write('set \\3 y1\n')
      fp.write('set \\4 y2\n')
      _panelobj.set(limits=True)
   _optionsobj.update(fp,**args)
   if borderFlag:
      fp.write('move \\13 \\15\n')
      fp.write('draw \\14 \\15\n')
      fp.write('draw \\14 \\16\n')
      fp.write('draw \\13 \\16\n')
      fp.write('draw \\13 \\15\n')
   else:
      if args.has_key('style') and args['style'] == '--':
         routine = 'fast' # must set to fast to get dashed contours to show up
      elif _optionsobj.lstyle == '--':
         routine = 'fast'
      if routine == 'smooth':
         fp.write('contour t\n')
      elif routine == 'fast':
         fp.write('contour s\n')
      elif routine == 'neg': # draw negative contours with same lstyle as + ones
         fp.write('contour -t\n')
      else:
         _error('contour(): Invalid routine keyword.  Try fast, smooth, or neg!')
   _optionsobj.reset(fp,**args)
   _panelobj.set(image=image,header=header)
   fp.close()

def curve(**args):
   '''Add a curve to the list plotted by legend().

      A curve is a dictionary that is set by the plot command and can
      be used with the legend command to make it easier to make legends.  This
      allows you to add an extra curve to those listed by legend() without
      having to call plot().

      Allowed optional **args:
         color - the color for the data
         size  - The size for each data point
         style - line style for the data
         width - the thickness of the line for drawing the curve
         text  - Text to describe the curve
         fillcolor - Color to fill symbols with'''

   c = {'color' : _colors[int(_optionsobj.color)], 'size' : _optionsobj.size,
        'style' : _lstyles[int(_optionsobj.lstyle)-1],
        'width' : _optionsobj.lwidth, 'text' : 'Generic Curve',
        'fillcolor' : _optionsobj.bg,
        'fillsize' : _optionsobj.size,
        'fillstyle' : _lstyles[int(_optionsobj.lstyle)-1]}

   if args.has_key('style') and args['style'] == None: # don't add style=None
      return                                           # to legend
   for k in args.keys():
      if k == 'text':
         if args['text']: # not set to None
            c[k] = _translatelatex(args['text'])
         else:
            return # don't add to list of curves for legend
      else:
         c[k] = str(args[k])
   # to properly set fill factor requires all other args to be parsed first
   if args.has_key('fillcolor'):
      fillstyle,fillfactor = _translatefillsymbol(args['style'])
      c['fillcolor'] = c['fillcolor']
      c['fillsize']  = fillfactor*float(c['size'])
      c['fillstyle'] = fillstyle
   _panelobj.set(curve=c)

def default(**args):
   '''Set default parameters that will be used for everything in the plot.

      Allowed optional **args:
         color  - a string giving the default color
         font   - a string giving the default font
         size   - a number giving the default size for everything
         style  - a string giving the default line style
         width  - a number giving the default width of lines
         bg     - a string giving the background color for text.  Use 't' for
                  transparent (the default).'''
   fp = _wipopen('default')
   _optionsobj.default(fp,**args)
   fp.close()

def errorbar(xcol,ycol,datafile=None,xerr=None,yerr=None,**args):
   '''Draw x,y errorbars on some data.

      xcol,ycol - Integer or list/tuple of x/y data
      datafile  - If a string, read columns xcol and ycol from datafile
      xerr      - Integer or list/tuple of x errorbars.  If int, read specified
                  column from datafile
      yerr      - Integer or list/tuple of y errorbars.  If int, read specified
                  column from datafile
      Allowed optional **args:
         color     - A string specifying the color of the errorbars
         size      - A number specifying the size of the caps on the errorbars
         style     - A string specifying the line style
         width     - A number specifying the line thickness
         limits    - If a list/tuple, use as limits.  Otherwise try to use
                     any prexisting limits or set new ones
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''

   fp = _wipopen('errorbar')
   ecol = 3 # column number for errorbar
   if not datafile:
      if _isseq(xcol) and _isseq(ycol):
         datafile = _maketempfile(xcol,ycol,xerr=xerr,yerr=yerr)
      else:
         _error('errorbar(): You must specify an input data file for errorbar!')
   else:
      datafile = _maketempfile(xcol,ycol,datafile,xerr,yerr)

   _panelobj.set(**args)
   _optionsobj.update(fp,**args)
   fp.write('data %s\n' %datafile)
   fp.write('xcol 1\n')
   fp.write('ycol 2\n')
   if _panelobj.get('logx'): fp.write('log x\n')
   if _panelobj.get('logy'): fp.write('log y\n')
   _panelobj.writelimits(fp,**args)
   if xerr:
      fp.write('ecol %d\n' %ecol)
      if _panelobj.get('logx'):
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
      if _panelobj.get('logy'):
         fp.write('log err\n')
         fp.write('errorbar 2\n')
         ecol = ecol + 1
         fp.write('ecol %d\n' %ecol)
         fp.write('log err\n')
         fp.write('errorbar 4\n')
      else:
         fp.write('errorbar 6\n')
   _optionsobj.reset(fp,**args)
   fp.close()

def halftone(image,header='rd',palette='gray',minmax=('immin','immax'),**args):
   '''Plot a halftone image with specified palette.

      Before using this, one should call winadj to make sure your pixels come
      out square.

      image   - string with name of image
      header  - string of header for image.  x and y can be different, e.g.
         'rd px', but if only one value is given, it will be the same for y
         as well.  The allowed values are:
            rd - Right Ascension/Declination
            so - Arcsecond offset
            mo - Arcminute offset
            po - pixel offset
            px - Absolute pixels
            gl - General linear coordinates
            go - General linear coordinate offsets
      palette - color palette for halftone
      minmax  - list or tuple of (min,max) halftone limits to display.  The
                default is the minimum and maximum of the image.
      Allowed optional **args:
         limits - The standard limits keyword'''

   fp = _wipopen('halftone')
   pal = _translatepalette(palette)
   if _panelobj.get('image') != image:
      _readimage(fp,image)
   fp.write('header %s\n' %header)
   fp.write('palette %s\n' %pal)
   if args.has_key('limits'):
      _panelobj.writelimits(fp,**args)
   fp.write('halftone ')
   if _isseq(minmax) and len(minmax) == 2:  # convert to string of two values,
      junkminmax = ' '.join(map(str,minmax))# one or both of which can be
      fp.write('%s\n' %junkminmax)           # immin or immax
   else:
      _error('halftone(): minmax must be given as a list/tuple of two values!')
   fp.write('set \\1 x1\n') # store map limits in registers 1-4
   fp.write('set \\2 x2\n') # useful for contour overlays
   fp.write('set \\3 y1\n')
   fp.write('set \\4 y2\n')
   if minmax:
      fp.write('set \\5 %s\n' %str(minmax[0]))
      fp.write('set \\6 %s\n' %str(minmax[1]))
   else:
      fp.write('set \\5 immin\n')
      fp.write('set \\6 immax\n')
   _panelobj.set(image=image,limits=True,header=header)
   fp.close()

def legend(x,y,height=2,length=2,**args):
   '''Make an entire legend box.

      x      - x location of upper left corner as a fraction
               (e.g. 0.5 would be centered)
      y      - y location of upper left corner as a fraction
      height - vertical skip (in char. units) between entries
      length - horizontal space (in char. units) for line/symbol

      Allowed options **args:
         color - a string giving the color for the text
         font  - a string giving the font to use for the text
         size  - a number giving the size for text
         style - a string giving the line style for the text
         width - a number giving the thickness of the line
         bg    - color for background of text.  Default is transparent'''

   fp = _wipopen('legend')
   fp.write('new tmpxlin tmpxsym tmpxlen tmpylin tmpxtxt tmpytxt\n')
   fp.write('new dx dy length height charheight\n')
   # I sort of determined by trail and error that 0.012/(vy2 - vy1) would
   # be the character height.  But then I found I need a fudge factor for dy.
   # oh, well.  These seem to work no matter what, even if they are kludgey.
   fp.write('limits 0 1 0 1\n')
   if not args.has_key('size'):
      fp.write('set charheight %s * 0.012 / (vy2 - vy1)\n' %_optionsobj.size)
   else:
      fp.write('set charheight %f * 0.012 / (vy2 - vy1)\n' %args['size'])
   # space between line and text
   fp.write('set dx 0.8 * charheight\n')
   # vertical offset of line/symbol from text
   fp.write('set dy 0.4 * charheight\n')
   # length of line
   fp.write('set length %f * charheight\n' %length)
   # vertical skip for each entry in legend
   fp.write('set height %f * charheight\n' %height)
   # starting x coordinate for symbol
   fp.write('set tmpxsym %g + (length / 2.0)\n' %x)
   # end x coordinate for line
   fp.write('set tmpxlen %g + length\n' %x)
   # starting y coordinate for text
   fp.write('set tmpytxt %g - height\n' %y)
   # y coordinate for line
   fp.write('set tmpylin tmpytxt + dy\n')
   # starting x coordinate for text
   fp.write('set tmpxtxt %g + length + dx\n' %x)
   #fp.write('echo vx1 vx2 vy1 vy2 tmpxlin tmpytxt\n')
   for c in _panelobj.curves: # put each curve in the legend
      sym = c['style']
      # First do the symbol or line for the entry
      if sym in _lstyles: # draw a line
         fp.write('move %g tmpylin\n' %x) # starting x/y coordinates
         _optionsobj.update(fp,**c)
         fp.write('draw tmpxlen tmpylin\n')
         _optionsobj.reset(fp,**c)
      else: # put individual symbol
         fp.write('move tmpxsym tmpylin\n')
         _optionsobj.update(fp,**c)
         fp.write('symbol %s\n' %_translatesymbol(c['style']))
         fp.write('dot\n')
         _optionsobj.reset(fp,**c)
         if c['fillcolor'] != '-1':
            _optionsobj.update(fp,color=c['fillcolor'],size=c['fillsize'],width=c['width'])
            fp.write('symbol %s\n' %_translatesymbol(c['fillstyle']))
            fp.write('dot\n')
            _optionsobj.reset(fp,color=c['fillcolor'],size=c['fillsize'],width=c['width'])
      # Now do the text label for the entry
      _optionsobj.update(fp,**args)
      fp.write('move tmpxtxt tmpytxt\n')
      fp.write('putlabel 0.0 %s\n' %c['text'])
      _optionsobj.reset(fp,**args)
      fp.write('set tmpylin tmpylin - height\n')
      fp.write('set tmpytxt tmpytxt - height\n')
   fp.write('free tmpxlin tmpxsym tmpxlen tmpylin tmpxtxt tmpytxt\n')
   fp.write('free dx dy length height charheight\n')
   fp.close()
   _panelobj.curves = [] # reset curves to nothing after a legend is made

def panel(idx,**args):
   '''Switch to panel idx.

      The nx,ny (and optionally gapx,gapy, and start) parameters should only
      be set for the first call to panel().  Thereafter, those values will be
      remembered.

      There are nx,ny panels in a grid.  The space between panels in the
      x and y directions is controlled by gapx and gapy.  By default, panels
      are counted from the top left going across, then down.  To start at
      the bottom left going across and up, set the start keyword to 'bottom'.


      idx   - Current panel number, (starts counting at 1)
      Optional **args:
         nx    - Number of panels in x direction (defaults to 1)
         ny    - Number of panels in y direction (defaults to 1)
         gapx  - space between panels in the x direction (defaults to 2)
         gapy  - space between panels in the y direction (defaults to 2)
         start - Panel number 1 will be at the top-left ('top') or bottom-left
                 ('bottom').  Defaults to 'top' '''

   fp = _wipopen('panel')
   if args.has_key('nx') and args.has_key('ny'): # start new panels
      _panelobj.resize(**args)
      fp.write('set xsubmar %f\n' %_panelobj.gapx)
      fp.write('set ysubmar %f\n' %_panelobj.gapy)
   elif args.has_key('nx') or args.has_key('ny'):
      _error('panel(): you must specify nx and ny!')
   if idx not in range(1,_panelobj.nx*_panelobj.ny+1):
      _error('panel(): idx must be between 1 and nx*ny!')
   _panelobj.idx = idx - 1
   if _panelobj.start == 'top':
      fp.write('panel %d %d %d\n' %(_panelobj.nx,_panelobj.ny,-1*idx))
   else:
      fp.write('panel %d %d %d\n' %(_panelobj.nx,_panelobj.ny,idx))
   fp.write('color %s\n'  %_optionsobj.color)
   fp.write('font %s\n'   %_optionsobj.font)
   fp.write('expand %s\n' %_optionsobj.size)
   fp.write('lstyle %s\n' %_optionsobj.lstyle)
   fp.write('lwidth %s\n' %_optionsobj.lwidth)
   fp.write('bgci %s\n'   %_optionsobj.bg)
   fp.close()

def plot(xcol,ycol,datafile=None,**args):
   '''Plot data from a file or input lists/tuples.

      This function is extremely versatile and can be used to plot data
      points and lines.  Note that when specifying column numbers from a file,
      columns are counted starting from 1.

      xcol,ycol - either an integer, list, or tuple for x/y data points.  If
                  integers, these are column numbers to read from the specified
                  datafile.
      datafile  - string name of input data file.  Leave as None if
                  xcol and ycol are a sequence of numbers
      Allowed options **args:
         color     - If a string, use as the color for every point.  If an
                     integer, read that column from the datafile for color
                     index for each point.
         size      - The size for each data point.
         style     - If a string, use as the symbol or line style.  If an
                     integer, then read from datafile for symbol for each point.
         width     - Line width
         fillcolor - Color to fill symbols with.  Only available for five-point
                     stars, squares, circles, and triangles.  If used
                     inappropriately, an error will occur.  Use in the same way
                     as for color keyword.  You can specify an integer, which
                     will then read the specified color from the datafile for
                     each point.
         limits    - If None, set min/max to values in xcol and ycol.  If a list
                     or tuple of xmin,xmax,ymin,ymax, set to specified values.
                     If anything else, do not attempt to set any limits (i.e.
                     assume they have already been set to something sensible)
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.
         text      - A string that can be used for the legend command.  Defaults
                     to "Generic Curve".  Set to None if you don't want to
                     add this curve to the legend'''

   eFlag = False # Set to true if we are reading a column from the file for
                 # a color for each point
   sFlag = False # Set to true if we are reading a column from the file for
                 # a symbol for each point

   fp = _wipopen('plot')
   if args.has_key('fillcolor'):
      fillcolor = args['fillcolor']
   else:
      fillcolor = None
   if not args.has_key('style'): # defaults is default line style
      args['style'] = _lstyles[int(_optionsobj.lstyle) - 1]
   if not args.has_key('width'):
      args['width'] = _optionsobj.lwidth

   if fillcolor is not None:
      if isinstance(args['style'],str):
         fillstyle,fillfactor = _translatefillsymbol(args['style'])
      if args.has_key('size'):              # fill size must be larger because
         fillsize = fillfactor*args['size'] # filled symbols are smaller than
      else:                                 # regular versions
         fillsize = fillfactor*float(_optionsobj.size)

   curve(**args) #stuff for a legend
   if datafile is None:
      if _isseq(xcol) and _isseq(ycol):
         num = len(xcol)
         if num > 10 or not args.has_key('limits'):
            datafile = _maketempfile(xcol,ycol)
            xcol = 1
            ycol = 2
         else:
            _plotpoints(fp,xcol[:],ycol[:],**args)
            if fillcolor is not None:
               _plotpoints(fp,xcol[:],ycol[:],color=fillcolor,size=fillsize,
                  style=fillstyle,width=args['width'])
      else:
         _error('plot(): You must specify a datafile!')
   else:
      num = _count(datafile)
   if num == 0: # datafile is empty
      return

   if datafile is not None:
      if xcol == 0 or ycol == 0: # make sure user doesn't enter zero-based numbers
         _error('plot(): Column numbers start at 1, not zero!')
      _panelobj.set(**args)
      fp.write('data %s\n' %datafile)
      fp.write('xcol %d\n' %xcol)
      fp.write('ycol %d\n' %ycol)
      if _panelobj.get('logx'): fp.write('log x\n')
      if _panelobj.get('logy'): fp.write('log y\n')
      _panelobj.writelimits(fp,**args)
      _optionsobj.update(fp,**args)
      if args.has_key('color') and isinstance(args['color'],int):
         eFlag = True
         fp.write('ecol %d\n' %args['color'])
      if isinstance(args['style'],int):
         fp.write('pcol %d\n' %args['style'])
         sym = '1'
         sFlag = True
      elif args['style'] == None: # skip actual plotting if style=None
         return
      else:
         sym = _translatesymbol(args['style'])
         if sym == '99': # not a symbol so see if it is a line style
            line = _translatelstyle(args['style'])
      if sym != '99': # plot a symbol instead of a line
         if not sFlag: # i.e. don't read symbols from datafile
            fp.write('symbol %s\n' %sym)
         if eFlag: # read colors for each point from the file
            fp.write('points 1\n')
         else:
            fp.write('points\n')
         _optionsobj.reset(fp,**args)
         if fillcolor:
            fp.write('symbol %s\n' %_translatesymbol(fillstyle))
            if isinstance(fillcolor,int):
               _optionsobj.update(fp,size=fillsize)
               fp.write('ecol %d\n' %fillcolor)
               fp.write('points 1\n')
            else:
               _optionsobj.update(fp,color=fillcolor,size=fillsize)
               fp.write('points\n')
            _optionsobj.reset(fp,color=fillcolor,size=fillsize)
      else: # plot a line
         fp.write('lstyle %s\n' %line)
         fp.write('connect\n')
         _optionsobj.reset(fp,**args)
   fp.close()

def poly(xcol,ycol,datafile=None,fill='s',**args):
   '''Draw a closed polygon.

      xcol,ycol - either an integer, list, or tuple for x/y data points
      datafile  - string name of input data file.  Leave as None if
                  xcol and ycol are a sequence of numbers
      fill      - Fill style for polygon, defaults to solid
      Allowed optional **args:
         color     - string with color to use for line style
         style     - string with line style to draw polygon with
         width     - Line width of polygon
         fillcolor - color to fill polygon with.  Defaults to color.
         limits    - If None, set min/max to values in xcol and ycol.  If a list
                     or tuple of xmin,xmax,ymin,ymax, set to specified values.
                     If anything else, do not attempt to set any limits (i.e.
                     assume they have already been set to something sensible)
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''

   fp = _wipopen('poly')
   if not datafile:
      datafile = _maketempfile(xcol,ycol)
      xcol = 1
      ycol = 2
   if fill != 'h':
      if args.has_key('fillcolor'):
         pass
      elif args.has_key('color'):
         args['fillcolor'] = args['color']
      else:
         args['fillcolor'] = _colors[int(_optionsobj.color)]

   _panelobj.set(**args)
   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   if _panelobj.get('logx'): fp.write('log x\n')
   if _panelobj.get('logy'): fp.write('log y\n')
   _panelobj.writelimits(fp,**args)
   _optionsobj.update(fp,**args)
   if args.has_key('fillcolor'):
      _optionsobj.update(fp,color=args['fillcolor'])
      fillstyle = _translatefill(fill)
      fp.write('fill %s\n' %fillstyle)
      fp.write('poly\n')
      _optionsobj.update(fp,color=args['fillcolor'])
   _optionsobj.update(fp,**args)
   fp.write('fill %s\n' %_translatefill('h'))
   fp.write('poly\n')
   _optionsobj.reset(fp,**args)
   fp.close()

def rect(xmin,xmax,ymin,ymax,fill='s',**args):
   '''Draw a rectangle on the figure.

      This function uses the poly() command behind the scenes.

      xmin,xmax,ymin,ymax - the limits of the rectangle
      fill                - fill style for rectangle
      Allowed optional **args:
         color     - string with color to use for line style
         style     - string with line style to draw rectangle with
         width     - thickness of line used to draw rectangle
         fillcolor - color to fill polygon with
         limits    - If a list/tuple, use as limits.  Otherwise try to use
                     any prexisting limits or set new ones
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''

   poly([xmin,xmax,xmax,xmin],[ymin,ymin,ymax,ymax],fill=fill,**args)

def savefig(filename,orient='portrait',color=True,debug=False):
   '''Make the output plot by actually running wip.

      filename - a string or list/tuple giving the output filename(s) with
                 .gif or .ps extension (files) or .xs for an xwindow (e.g. 1.xs).
      orient   - make plot portrait or landscape orientation?
      color    - set to False to make a black and white plot
      debug    - set to True if you do not want to delete all the temp files needed by wip'''

   global _wipfile,_tmplist
   if orient not in ('landscape','portrait'):
      _error('savefig(): invalid orient option in savefig.  Try landscape or portrait')

   if _isseq(filename):
      fileseq = filename
   else:
      fileseq = [filename]
   for f in fileseq:
      if f.endswith('.gif'):
         dev = '%s/gif' %f
      elif f.endswith('.ps'):
         dev = '%s/' %f
         if orient == 'portrait':
            dev = dev + 'v'
         if color:
            dev = dev + 'c'
         dev = dev + 'ps'
      elif f.endswith('.xs'):
         dev = '%s/xs' %f[:-3]
      else:
         _error('savefig(): Invalid output plot filename suffix.  Try .ps or .gif')
      if _wipfile != '???':
         os.system('wip -x -d %s %s' %(dev,_wipfile))
   if not debug:
      os.remove(_wipfile)
      for f in _tmplist:
         os.remove(f)
   _wipfile = '???'
   _tmplist = []

def stick(xcol,ycol,anglecol,lengthcol,datafile=None,scale=1,start=90,
   align='left',**args):
   '''Draw a stick field (i.e. a polarization map, which doesn't really have
      a direction like vectors do).

      lengthcol is a column with the length of each arrow and anglecol is the
      angle for each arrow, in degrees.  Zero degrees is +x, and angles 
      increase counter-clockwise.

      xcol,ycol - either an integer, list, or tuple for x/y data points
      anglecol  - either an integer, list, or tuple for direction of sticks.
                  Note, angles are expected to be degrees counter-clockwise from
                  the +x axis.  The normal convention for polarization is 
                  degrees counter-clockwise from North.
      lengthcol - either an integer, list, or tuple for length of sticks
      datafile  - string name of input data file.  Leave as None if
                  xcol,ycol,pcol,ecol are a sequence of numbers
      scale     - scale factor for length of sticks
      start     - starting angle from +x that corresponds to zero.  The default
                  of 90 means that zero degrees is the +y axis.  This is the
                  normal convention in polarization maps.
      align     - 'left' = xcol,ycol are ends of sticks, 'center' = xcol,ycol
                  are center of sticks, 'right' = xcol,ycol are tips of sticks.
      Allowed optional **args:
         color     - string with color for each stick
         width     - Line width of sticks
         limits    - If None, set min/max to values in xcol and ycol.  If a list
                     or tuple of xmin,xmax,ymin,ymax, set to specified values.
                     If anything else, do not attempt to set any limits (i.e.
                     assume they have already been set to something sensible)'''

   fp = _wipopen('stick')
   if not datafile:
      n1 = len(xcol)
      n2 = len(ycol)
      n3 = len(anglecol)
      n4 = len(lengthcol)
      if n1 != n2 or n1 != n3 or n1 != n4:
         _error('stick(): xcol, ycol, anglecol, and lengthcol must all have the same number of elements!')
      if align == 'left':
         x0 = xcol
         y0 = ycol
      else:
         x0 = [0 for i in xrange(n1)]
         y0 = [0 for i in xrange(n3)]
      x1 = [0 for i in xrange(n2)]
      y1 = [0 for i in xrange(n4)]
      for i in xrange(n1):
         a = math.pi/180.0*(start + anglecol[i])
         if align == 'center':
            l = scale*lengthcol[i]/2.0
         else:
            l = scale*lengthcol[i]
         if align != 'left':
            x0[i] = xcol[i] - l*math.cos(a)
            y0[i] = ycol[i] - l*math.sin(a)
         x1[i] = xcol[i] + l*math.cos(a)
         y1[i] = ycol[i] + l*math.sin(a)
   else:
      fp2 = open(datafile,'r')
      x0 = []
      y0 = []
      x1 = []
      y1 = []
      for line in fp2:
         if line[0] != '#':
            t = line.split()
            x = float(t[xcol-1])
            y = float(t[ycol-1])
            a = math.pi/180.0*(start + float(t[anglecol-1]))
            if align == 'center':
               l = scale*float(t[lengthcol-1])/2.0
            else:
               l = scale*float(t[lengthcol-1])
            if align == 'left':
               x0.append(x)
               y0.append(y)
            else:
               x0.append(x - l*math.cos(a))
               y0.append(y - l*math.sin(a))
            x1.append(x + l*math.cos(a))
            y1.append(y + l*math.sin(a))
      fp2.close()

   _panelobj.set(**args)
   if _panelobj.get('header') == 'rd':
      fp.write('header px\n')
      _panelobj.set(header='px')
   _panelobj.writelimits(fp,**args)
   _optionsobj.update(fp,**args)
   for i in xrange(len(x0)): # draw the sticks
      fp.write('move %f %f\n' %(x0[i],y0[i]))
      fp.write('draw %f %f\n' %(x1[i],y1[i]))
   _optionsobj.reset(fp,**args)
   fp.close()

def text(x,y,text,align='left',angle=0,**args):
   '''Put a given text label at specified coordinates.

      x,y       - coordinates for location of text
      text      - a string of text to print out
      align     - alignment for label.  Either left, center, or right
      angle     - angle in degrees for text
      Allowed options **args:
         color     - a string giving the color for the label
         font      - a string giving the font to use for the label
         size      - a number giving the size for label
         style     - a string giving the line style for the label
         width     - a number giving the width of the lines
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.
         bg        - background color for text.  Default is -1 (transparent)'''
   fp = _wipopen('text')
   _panelobj.set(**args)
   if _panelobj.get('logx'):
      xtmp = math.log10(_translatecoords(x,coord='ra'))
   else:
      xtmp = _translatecoords(x,coord='ra')
   if _panelobj.get('logy'):
      ytmp = math.log10(_translatecoords(y,coord='dec'))
   else:
      ytmp = _translatecoords(y,coord='dec')
   al = _translatealign(align)
   _optionsobj.update(fp,**args)
   if angle != 0:
      fp.write('angle %s\n' %angle)
   fp.write('move %f %f\n' %(xtmp,ytmp))
   fp.write('putlabel %s %s\n' %(al,_translatelatex(text)))
   _optionsobj.reset(fp,**args)
   if angle != 0:
      fp.write('angle 0\n')
   fp.close()

def title(text,offset=0,align='center',**args):
   '''Set the title of the plot to the given text.

      text   - a string of text to print out as title of plot
      offset - offset for text in addition to standard offset for a title
      align  - alignment for label.  Either left, center, or right, or a
               number between zero and one. (zero=left, one=right).
      Allowed optional **args:
         color  - a string giving the color for the title
         font   - a string giving the font to use for the title
         size   - a number giving the size for the title
         style  - a string giving the line style for the title
         width  - a number giving the width of the lines
         bg     - background color for text'''
   fp = _wipopen('title')
   _mtext(fp,text,offset,align,side='top',**args)
   fp.close()

def vector(xcol,ycol,anglecol,lengthcol,datafile=None,taper=45,vent=0.0,
   fill='s',align='left',**args):
   '''Draw a vector field.

      lengthcol is a column with the length of each arrow and anglecol is the
      angle for each arrow, in degrees.  Zero degrees +x, and angles increase
      counter-clockwise.

      xcol,ycol - either an integer, list, or tuple for x/y data points
      anglecol  - either an integer, list, or tuple for direction of arrows.
                  Note, angles are expected to be degrees counter-clockwise from
                  the +x axis.  The normal convention for polarization is 
                  degrees counter-clockwise from North.
      lengthcol - either an integer, list, or tuple for length of arrows
      datafile  - string name of input data file.  Leave as None if
                  xcol,ycol,pcol,ecol are a sequence of numbers
      taper     - tapering angle, in degrees, for the arrowheads
      vent      - fraction of each arrowhead that is cut-away in the back.
      fill      - fill style for arrow heads
      align     - 'left' = xcol,ycol are ends of arrows, 'center' = xcol,ycol
                  are center of arrows, 'right' = xcol,ycol are tips of arrows.
                  Currently, it seems that changing this from the default only
                  works for RA,DEC world coordinate systems.
      Allowed optional **args:
         color     - string with color for each arrow
         size      - Size of arrows
         width     - Line width of arrows
         limits    - If None, set min/max to values in xcol and ycol.  If a list
                     or tuple of xmin,xmax,ymin,ymax, set to specified values.
                     If anything else, do not attempt to set any limits (i.e.
                     assume they have already been set to something sensible)
         logx,logy - If True, make logarithmic in x/y direction.  Otherwise
                     default to what has already been set for plot/panel.'''

   global _tmplist
   fp = _wipopen('vector')
   fillstyle = _translatefill(fill)
   if not datafile:
      n1 = len(xcol)
      n2 = len(ycol)
      n3 = len(anglecol)
      n4 = len(lengthcol)
      if n1 != n2 or n1 != n3 or n1 != n4:
         _error('vector(): In vector, xcol, ycol, anglecol, and lengthcol must all have the same number of elements!')
      datafile = tempfile.mktemp()
      _tmplist.append(datafile)
      fp2 = open(datafile,'w')
      for i in range(len(xcol)):
         fp2.write('%4.4e  %4.4e  %4.4e  %4.4e\n' %(_translatecoords(xcol[i],'ra'),
            _translatecoords(ycol[i],'dec'),anglecol[i],lengthcol[i]))
      fp2.close()
      xcol      = 1
      ycol      = 2
      anglecol  = 3
      lengthcol = 4
   if align == 'center' or align == 'right':      
      junkfile = tempfile.mktemp()
      _tmplist.append(junkfile)
      fp2 = open(datafile,'r')
      fp3 = open(junkfile,'w')
      for line in fp2:
         if line[0] != '#':
            t = line.split()
            x = float(t[xcol-1])
            y = float(t[ycol-1])
            l = float(t[lengthcol - 1])
            a = float(t[anglecol - 1])
            if align == 'center':
               x = x - 0.5*l*math.cos(math.pi/180.0*a)
               y = y - 0.5*l*math.sin(math.pi/180.0*a)
            elif align == 'right':
               x = x - l*math.cos(math.pi/180.0*a)
               y = y - l*math.sin(math.pi/180.0*a)
            fp3.write('%4.4e  %4.4e  %4.4e  %4.4e\n' %(x,y,a,l))
      xcol      = 1
      ycol      = 2
      anglecol  = 3
      lengthcol = 4
      fp2.close()
      fp3.close()
      datafile = junkfile

   _panelobj.set(**args)
   if _panelobj.get('header') == 'rd':
      fp.write('header px\n')
      _panelobj.set(header='px')
   fp.write('data %s\n' %datafile)
   fp.write('xcol %d\n' %xcol)
   fp.write('ycol %d\n' %ycol)
   fp.write('ecol %d\n' %anglecol)
   fp.write('pcol %d\n' %lengthcol)
   if _panelobj.get('logx'): fp.write('log x\n')
   if _panelobj.get('logy'): fp.write('log y\n')
   _panelobj.writelimits(fp,**args)
   _optionsobj.update(fp,**args)
   fp.write('fill %s\n' %fillstyle)
   fp.write('vector %f %f\n' %(taper,vent))
   _optionsobj.reset(fp,**args)
   fp.close()

def viewport(xmin,xmax=None,ymin=None,ymax=None):
   '''Use this to set the plot area.

      Sometimes WIP gets this wrong and part of your plot is off the page.
      winadj can alter these values, but only within the bounds of the viewport.

      xmin - either a number or a list/tuple of four numbers
      xmax - leave as None if xmin is a list/tuple
      ymin - leave as None if xmin is a list/tuple
      ymax - leave as None if xmin is a list/tuple'''
   fp = _wipopen('viewport')
   if _isseq(xmin):
      if len(xmin) != 4:
         _error('viewport(): You must specify four limits as xmin,xmax,ymin,ymax!')
      limits = tuple(xmin)
   else:
      try:
         limits = map(float,(xmin,xmax,ymin,ymax))
      except ValueError:
         _error('viewport(): You must specify numbers for xmin,xmax,ymin,ymax!')
   fp.write('viewport %f %f %f %f\n' %tuple(limits))
   fp.close()

def wedge(side='right',offset=1,thickness=1,minmax=None,**args):
   '''Draw a halftone wedge on the image.

      This is only useful for halftone images, though you could draw a wedge
      without first calling halftone(), if you felt perverse.  Note that in
      that case, you cannot set the palette.

      side      - string containing side to draw wedge on.  One of left, right,
                  top, or bottom
      offset    - offset of wedge from side
      thickness - thickness of wedge
      minmax    - a list/tuple of the min/max values for the wedge.  Defaults
                  to whatever min/max was for halftone
      Allowed optional **args:
         all the axis() options.  Too many to list here; see the User's Manual.
         color     - color for the box around the wedge
         font      - font for labelling the wedge
         size      - size of box labels around wedge
         style     - line style for box
         width     - thickness of box around wedge'''
   if side not in ('bottom','top','left','right'):
      _error('wedge(): unknown side parameter: %s!' %side)

   fp = _wipopen('wedge')
   _optionsobj.update(fp,**args)
   if not args.has_key('box'):
      args['box'] = ('bottom','left','top','right')
   if args.has_key('drawtickx') and not args['drawtickx']: # exists and False
      args['majortickx'] = False
   else:
      if not args.has_key('majortickx'): args['majortickx'] = True
   if args.has_key('drawticky') and not args['drawticky']: # exists and False
      args['majorticky'] = False
   else:
      if not args.has_key('majorticky'): args['majorticky'] = True
   if not args.has_key('number'):
      args['number'] = [side]
   xlab,ylab = _translateaxis(**args)
   if side in ('left','right'):
      label = ylab
   else:
      label = xlab
   fp.write('wedge %s %f %f ' %(side[0],offset,thickness))
   if minmax:
      fp.write('%g %g %s\n' %(minmax[0],minmax[1],label))
   else:
      fp.write('\\5 \\6 %s\n' %label)
   _optionsobj.reset(fp,**args)
   fp.close()

def winadj(xfac=1,yfac=1,image=None):
   '''Adjust the window so the plot will have the given aspect ratio.

      Note, for convenience sake, if you want to adjust a window size for
      an image, you can just call winadj(imagename) instead of
      winadj(image=imagename).

      xfac   - Relative size of x dimension
      yfac   - Relative size of y dimension
      image  - If a string of an image name (and optionally a subimage defined
               ala IRAF's method : image[xmin:xmax,ymin:ymax], then adjust
               window according to image size, taking into account xfac
               and yfac.'''
   fp = _wipopen('winadj')
   if isinstance(xfac,str): # xfac is a string, so assume it is an image
      img = xfac
      xfac = 1
   else:
      img = image
   if img:
      _panelobj.set(image=img)
      blah = _readimage(fp,img)
      if blah: # a subimage was defined by the user
         nx = xfac*(blah[1] - blah[0] + 1)
         ny = yfac*(blah[3] - blah[2] + 1)
         fp.write('set \\7 %d\n' %nx)
         fp.write('set \\8 %d\n' %ny)
      else: # no subimage so use whole image.  WIP knows about nx and ny
         fp.write('set \\7 %g * nx\n' %xfac)
         fp.write('set \\8 %g * ny\n' %yfac)
      fp.write('winadj 0 \\7 0 \\8\n')
   else:
      fp.write('winadj 0 %g 0 %g\n' %(xfac,yfac))
      fp.write('set \\7 %g\n' %xfac)
      fp.write('set \\8 %g\n' %yfac)
   fp.close()

def xlabel(text,offset=0,align='center',**args):
   '''Set the x label to the given text.

      text   - a string of text to print out as x axis label
      offset - offset for text in addition to standard offset for an x label
      align  - alignment for label.  Either left, center, or right, or a
               number between zero and one. (zero=left, one=right).
      Allowed optional **args:
         color  - a string giving the color for the label
         font   - a string giving the font to use for the label
         size   - a number giving the size for label.
         style  - a string giving the line style for the label
         width  - a number giving the width of the lines
         bg     - background color for text'''
   fp = _wipopen('xlabel')
   _mtext(fp,text,offset,align,side='bottom',**args)
   fp.close()

def ylabel(text,offset=0,align='center',side='left',**args):
   '''Set the y label to the given text.

      text   - a string of text to print out as y axis label
      offset - offset for text in addition to standard offset for a y label
      align  - alignment for label.  Either left, center, or right, or a
               number between zero and one. (zero=left, one=right).
      side   - can be left or right to put a label on left/right sides
      Allowed options **args:
         color  - a string giving the color for the label
         font   - a string giving the font to use for the label
         size   - a number giving the size for label.
         style  - a string giving the line style for the label
         width  - a number giving the width of the lines
         bg     - background color for text'''
   fp = _wipopen('ylabel')
   _mtext(fp,text,offset,align,side,**args)
   fp.close()

### Past this point are functions the user should never access directly ###

class _options:
   def __init__(self):
      # Note, these are all defined in the way WIP would use them.
      self.font   = '2'   # default font (roman)
      self.lwidth = '1'   # default line width
      self.lstyle = '1'   # default line style (solid)
      self.color  = '1'   # default color (black)
      self.size   = '1'   # default size
      self.bg     = '-1'  # default background text color, i.e. transparent
   def update(self,fp,**sargs):
      '''Write out any options specified by the user'''
      for k in sargs.keys():
         if   k == 'color':
            if isinstance(sargs[k],str):
               fp.write('color %s\n' %_translatecolor(sargs[k]))
         elif k == 'font':  fp.write('font %s\n' %_translatefont(sargs[k]))
         elif k == 'size':  fp.write('expand %s\n' %sargs[k])
         elif k == 'style':
            if isinstance(sargs[k],str):
               sym = _translatesymbol(sargs[k]) # don't attempt for symbols
               if sym == '99':
                  sym = _translatelstyle(sargs[k])
                  fp.write('lstyle %s\n' %sym)
         elif k == 'width': fp.write('lwidth %s\n' %sargs[k])
         elif k == 'bg':    fp.write('bgci %s\n' %_translatecolor(sargs[k]))
   def reset(self,fp,**sargs):
      '''Reset any options changed by self.update to their defaults'''
      for k in sargs.keys():
         if   k == 'color':     fp.write('color %s\n' %self.color)
         elif k == 'fillcolor': fp.write('color %s\n' %self.color)
         elif k == 'font':      fp.write('font %s\n' %self.font)
         elif k == 'size':      fp.write('expand %s\n' %self.size)
         elif k == 'style':     fp.write('lstyle %s\n' %self.lstyle)
         elif k == 'width':     fp.write('lwidth %s\n' %self.lwidth)
         elif k == 'bg':        fp.write('bgci %s\n' %self.bg)
   def default(self,fp,**sargs):
      '''Change the default values'''
      for k in sargs.keys():
         if   k == 'color': self.color  = _translatecolor(sargs['color'])
         elif k == 'font':  self.font   = _translatefont(sargs['font'])
         elif k == 'size':  self.size   = str(sargs['size'])
         elif k == 'style': self.lstyle = _translatelstyle(sargs['style'])
         elif k == 'width': self.lwidth = str(sargs['width'])
         elif k == 'bg':
            if sargs[k] == 't': # t for transparent
               self.bg = '-1'
            else:
               self.bg = _translatecolor(sargs['bg'])
      self.update(fp,**sargs)

class _panel:
   def __init__(self):
      self.nx    = 0     # number of panels in x direction (set by self.resize)
      self.ny    = 0     # number of panels in y direction (set by self.resize)
      self.idx   = 0     # index of current panel number
      self.gapx  = 2     # space between panels in x
      self.gapy  = 2     # space between panels in y
      self.start = 'top' # control if numbering starts in top or bottom left
      self.limits  = []  # flag whether limits are set for each panel
      self.logx    = []  # flag whether logx is plotted
      self.logy    = []  # flag whether logy is plotted
      self.image   = []  # name of image, if any, for each panel
      self.header  = []  # header for each image (px, rd, etc)
      self.curves  = []  # curves to plot for legend
      self.resize(nx=1,ny=1,gapx=2,gapy=2,start='top')

   def get(self,key):
      '''Return the specified value for the current panel'''
      if   key == 'limits' : return self.limits[self.idx]
      elif key == 'logx'   : return self.logx[self.idx]
      elif key == 'logy'   : return self.logy[self.idx]
      elif key == 'image'  : return self.image[self.idx]
      elif key == 'header' : return self.header[self.idx]
      else: _warning('_panel(): Invalid key requested: %s' %key)

   def writelimits(self,fp,**args):
      """New function for limits"""
      if args.has_key('logx'): self.logx[self.idx] = args['logx']
      if args.has_key('logy'): self.logy[self.idx] = args['logy']
      if args.has_key('limits') and args['limits'] == None:
         del(args['limits'])
      if args.has_key('limits'):
         if args['limits'] == 'last': # use last set limits from other panel
            idx = self.idx - 1
            while idx >= 0:
               if self.limits[idx]:
                  self.limits[self.idx] = self.limits[idx]
                  self.logx[self.idx] = self.logx[idx]
                  self.logy[self.idx] = self.logy[idx]
                  break
               idx = idx - 1
         else:
            tmp = list(args['limits'])
            if self.logx[self.idx]:
               ## TODO: No warning for changing these values
               if tmp[0] == 0: tmp[0] = 1e-5
               if tmp[1] == 0: tmp[1] = 1e-5
               tmp[0] = math.log10(tmp[0])
               tmp[1] = math.log10(tmp[1])
            if self.logy[self.idx]:
               if tmp[2] == 0: tmp[2] = 1e-5
               if tmp[3] == 0: tmp[3] = 1e-5
               tmp[2] = math.log10(tmp[2])
               tmp[3] = math.log10(tmp[3])
            fp.write('set \\1 %g\n' %tmp[0])
            fp.write('set \\2 %g\n' %tmp[1])
            fp.write('set \\3 %g\n' %tmp[2])
            fp.write('set \\4 %g\n' %tmp[3])
            self.limits[self.idx] = True
         fp.write('limits \\1 \\2 \\3 \\4\n')
      elif self.limits[self.idx]: # limits already exist for this panel,
         pass                     # so reuse them
         #fp.write('limits \\1 \\2 \\3 \\4\n')
      else: # no limits set in this panel, so make new ones
         fp.write('limits\n')
         fp.write('set \\1 x1\n')
         fp.write('set \\2 x2\n')
         fp.write('set \\3 y1\n')
         fp.write('set \\4 y2\n')
         if args.has_key('reversex') and args['reversex']:
            fp.write('set \\1 x2\n')
            fp.write('set \\2 x1\n')
         if args.has_key('reversey') and args['reversey']:
            fp.write('set \\1 y2\n')
            fp.write('set \\2 y1\n')
         if self.logx[self.idx]:
            ## TODO: No warning for changing these values
            fp.write('if (\\1 == 0) set \\1 1e-5\n')
            fp.write('if (\\2 == 0) set \\2 1e-5\n')
         if self.logy[self.idx]:
            fp.write('if (\\3 == 0) set \\3 1e-5\n')
            fp.write('if (\\4 == 0) set \\4 1e-5\n')
         self.limits[self.idx] = True
         fp.write('limits \\1 \\2 \\3 \\4\n')

   def resize(self,**args):
      '''Change size of a panel, either newly-created, or from panel() cmd'''
      if args.has_key('gapx'):  self.gapx = args['gapx']
      if args.has_key('gapy'):  self.gapy = args['gapy']
      if args.has_key('start'): self.start = args['start']
      if args.has_key('nx'):
         nx = args['nx']
      else:
         nx = self.nx
      if args.has_key('ny'):
         ny = args['ny']
      else:
         ny = self.ny

      if self.start not in ('top','bottom'):
         _error('_panel(): start keyword must be top or bottom!')
      for i in range(self.nx*self.ny,nx*ny):
         self.limits.append(False)
         self.logx.append(False)
         self.logy.append(False)
         self.image.append(None)
         self.header.append(None)
      self.nx = nx
      self.ny = ny

   def set(self,**args):
      '''Set the specified value for the current panel'''
      for k,v in args.iteritems():
         if   k == 'logx'  : self.logx[self.idx] = v
         elif k == 'logy'  : self.logy[self.idx] = v
         elif k == 'image' : self.image[self.idx] = v
         elif k == 'header': self.header[self.idx] = v
         elif k == 'curve' : self.curves.append(v)
         elif k == 'limits': self.limits[self.idx] = v

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
      _error("_count(): datafile %s does not exist!" %datafile)
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

   return isinstance(var,(list,tuple))

def _maketempfile(xcol,ycol,datafile=None,xerr=None,yerr=None):
   '''Make a temporary data file for reading by wip.

      xcol  - either an integer (for a column from datafile) or a list/tuple
      ycol  - either an integer (for a column from datafile) or a list/tuple
      xerr  - either an integer (for a column from datafile) or a list/tuple
      yerr  - either an integer (for a column from datafile) or a list/tuple
      datafile - set to a filename to read data from that file'''
   global _tmplist

   logx = _panelobj.get('logx')
   logy = _panelobj.get('logy')
   if not datafile:
      n1 = len(xcol)
      n2 = len(ycol)
      if n1 != n2: _error('_maketempfile(): x and y arrays must be the same length!')
      if xerr:
         if len(xerr) != n1:
            _error('_maketempfile(): xerr array must have same length as x and y arrays!')
      if yerr:
         if len(yerr) != n1:
            _error('_maketempfile(): yerr array must have same length as x and y arrays!')
   elif not os.path.exists(datafile):
      _error('_maketempfile(): file %s does not exist for reading!' %datafile)

   blah = tempfile.mktemp()
   _tmplist.append(blah)
   fp2 = open(blah,'w')

   if datafile:
      fp1 = open(datafile,'r')
      line = fp1.readline()
      while line:
         if line[0] != '#':
            tmp = line.split()
            if _panelobj.get('image') and _panelobj.get('header') == 'rd':
               fp2.write('%6.6e %6.6e ' %(_translatecoords(tmp[xcol-1],'ra'),
                  _translatecoords(tmp[ycol-1],'dec')))
            else:
               fp2.write('%s %s ' %(tmp[xcol-1],tmp[ycol-1]))
            if xerr:
               _maketemphelper(fp2,float(tmp[xcol-1]),float(tmp[xerr - 1]),logx)
            if yerr:
               _maketemphelper(fp2,float(tmp[ycol-1]),float(tmp[yerr - 1]),logy)
            fp2.write('\n')
         line = fp1.readline()
      fp1.close()
   else:
      for i in range(len(xcol)):
         fp2.write('%6.6e %6.6e ' %(_translatecoords(xcol[i],'ra'),
            _translatecoords(ycol[i],'dec')))
         if xerr:
            _maketemphelper(fp2,float(xcol[i]),float(xerr[i]),logx)
         if yerr:
            _maketemphelper(fp2,float(ycol[i]),float(yerr[i]),logy)
         fp2.write('\n')
   fp2.close()
   return blah

def _maketemphelper(fp,value,error,logFlag):
   '''Helper function for _maketempfile that consolidates the code for making
      errorbars and log errorbars with WIP.  You have to do some extra
      gymnastics to make these happen in WIP.'''
   if logFlag:
      if value == 0:
         fp.write('1 ')
      else:
         fp.write('%6.6e ' %((value+error)/value))
      if value == error:
         fp.write('1 ')
      else:
         # when value-err < 0, this causes the errorbars to be drawn funny
         # due to taking the log of a negative number.  So, we fix
         # by forcing err, the errorbar value to be ~99% of value.  This
         # reduces the value/(value-err) to 99.
         if value - error < 0:
            fp.write('%6.6e ' %99)
         else:
            fp.write('%6.6e ' %(value/(value-error)))
   else:
      fp.write('%s ' %error)

def _mtext(fp,text,offset=0,align='center',side='top',**args):
   '''Combine stuff for using mtext, which is used by xlabel(), ylabel(),
      and title()
      text   - a string of text
      offset - offset for text in addition to standard offset (which depends
               on the chosen side).
      align  - alignment for label.  Either left, center, or right, or a
               number between zero and one. (zero=left, one=right).
      side   - put text on this side.  Options are left, right, top, bottom.
      Allowed optional **args:
         color  - a string giving the color for the title
         font   - a string giving the font to use for the title
         size   - a number giving the size for the title
         style  - a string giving the line style for the title
         width  - a number giving the width of the lines
         bg     - background color for text'''
   al = _translatealign(align)
   if side == 'top':
      off = str(2.0 + float(offset))
   elif side == 'left':
      off = str(2.2 + float(offset))
   elif side == 'right':
      off = str(2.2 + float(offset))
   elif side == 'bottom':
      off = str(3.2 + float(offset))
   else:
      _error('_mtext(): Side keyword must be one of: top, bottom left, right!')

   # doesn't seem to properly pick-up default parameters that are set, so
   # we force them to be written out. TODO: Still a problem?
   #_optionsobj.reset(fp,color=1,font=1,size=1,style=1,width=1,bg=1)
   # Now override defaults
   _optionsobj.update(fp,**args)
   fp.write('mtext %c %s 0.5 %s %s\n' %(side[0].upper(),off,al,_translatelatex(text)))
   _optionsobj.reset(fp,**args)

def _plotpoints(fp,xlist,ylist,**args):
   '''Plot data points from input lists of coordinates.

      This function is a helper to the plot command.  When there are less
      than 10 data points, plot will call this function rather than go through
      the process of making a temp file.  Like plot(), you can show points
      and lines.

      fp        - The file pointer where wip commands are written
      xlist,ylist - Lists or tuples with the x and y positions of points to
                  plot
      Allowed optional **args:
         color - If a string, use as the color for every point.  If an integer,
                 read that column from the datafile for color index for each
                 point.
         size  - The size for each data point.
         style - If a string, use as the symbol or line style.  If an integer,
                 then read from datafile for symbol for each point.
         width - Line width'''

   _panelobj.set(**args)
   _optionsobj.update(fp,**args)
   if _panelobj.get('logx'):
      for i in range(len(xlist)):
         try:
            xlist[i] = math.log10(xlist[i])
         except ValueError:
            _error("_plotpoints(): problem with taking log of %f" %xlist[i])
   if _panelobj.get('logy'):
      for i in range(len(ylist)):
         try:
            ylist[i] = math.log10(ylist[i])
         except ValueError:
            _error("_plotpoints(): problem with taking log of %f" %ylist[i])
   _panelobj.writelimits(fp,**args)
   if args.has_key('style'):
      if args['style'] == None: # skip plotting if style=None
         return
      else:
         sym = _translatesymbol(args['style'])
   else:
      sym = _translatesymbol('o')
   if sym == '99':
      line = _translatelstyle(args['style'])
      fp.write('lstyle %s\n' %line)
      fp.write('move %f %f\n' %(_translatecoords(xlist[0],'ra'),_translatecoords(ylist[0],'dec')))
      for i in range(1,len(xlist)):
         fp.write('draw %f %f\n' %(_translatecoords(xlist[i],'ra'),_translatecoords(ylist[i],'dec')))
      _optionsobj.reset(fp,**args)
   else:
      fp.write('symbol %s\n' %sym)
      for a,b in zip(xlist,ylist):
         fp.write('move %f %f\n' %(_translatecoords(a,'ra'),_translatecoords(b,'dec')))
         fp.write('dot\n')
      _optionsobj.reset(fp,**args)

def _readimage(fp,image):
   '''Perform image and subimage commands on a given image name.

      Return x and y pixel limits as a tuple.'''

   blah = re.findall(r'(\(|\[){1}',image)
   if len(blah) == 0:
      name = image
   else:
      name = image[:image.index(blah[0])]
   subimage = re.findall(r'\[.*\]',image) # get subimage pixels
   planenum = re.findall(r'\([0-9]*\)',image) # get plane number

   if len(planenum) > 1:
      _error('_readimage(): found more than one plane number!')
   elif len(planenum) == 1:
      planenum = int(planenum[0][1:-1])
   else:
      planenum = 1

   if os.path.exists(name):
      fp.write('image %s %d\n' %(name,planenum))
   else:
      _error('_readimage(): Image %s does not exist!' %name)

   if len(subimage) > 1:
      _error('_readimage(): found more than one subimage range!')
   elif len(subimage) == 1:
      blah = subimage[0][1:-1].split(',') #[1:-1] splits off [] at begin/end
      if len(blah) != 2:
         _error('_readimage(): You must specify image range as [xmin:xmax,ymin:ymax]!')
      try:
         blah = tuple(map(int,blah[0].split(':') + blah[1].split(':')))
      except ValueError:
         _error('_readimage(): Image range must be integer pixel values!')
      if len(blah) != 4:
         _error('_readimage(): You must specify image range as [xmin:xmax,ymin:ymax]!')
      fp.write('subimage %d %d %d %d\n' %blah)
      return blah
   else:
      return None

def _translatealign(align):
   '''Take useful alignment string and convert to wip format.'''
   if   align == 'left':    return '0.0'
   elif align == 'center':  return '0.5'
   elif align == 'right':   return '1.0'
   else:
      try:
         blah = float(align)
         if blah < 0 or blah > 1:
            _error('_translatealign(): Invalid alignment.  Try left,center,right, or a number')
         return align
      except ValueError:
         _error('_translatealign(): Invalid alignment.  Try left,center,right, or a number')
def _translateaxis(**args):
   """Convert **args into wip format box commands."""

   xaxis = ''
   yaxis = ''
   for k,v in args.iteritems():
      if k == 'box':
         for side in v:
            if   side == 'bottom':  xaxis += 'b'
            elif side == 'top':     xaxis += 'c'
            elif side == 'left':    yaxis += 'b'
            elif side == 'right':   yaxis += 'c'
            else: _error('_translateaxis(): unknown side for box: %s' %side)
      elif k == 'drawtickx': pass # if True, don't do anything since defaults
      elif k == 'drawticky': pass # to draw.  if False, again do nothing
      elif k == 'firstx':
         if not v: xaxis += 'f'
      elif k == 'firsty':
         if not v: yaxis += 'f'
      elif k == 'format':
         if len(v) != 2: _error('_translateaxis(): format must have two values!')
         if   v[0] == 'wcs':  xaxis += 'hz'
         elif v[0] == 'dec':  xaxis += '1'
         elif v[0] == 'exp':  xaxis += '2'
         elif v[0] == 'auto': pass

         else: _error('_translateaxis(): unknown format style: %s' %v[0])
         if   v[1] == 'wcs':  yaxis += 'dz'
         elif v[1] == 'dec':  yaxis += '1'
         elif v[1] == 'exp':  yaxis += '2'
         elif v[1] == 'auto': pass
         else: _error('_translateaxis(): unknown format style: %s' %v[1])
      elif k == 'gridx':
         if v: xaxis += 'g'
      elif k == 'gridy':
         if v: yaxis += 'g'
      elif k == 'logx':
         if v: xaxis += 'l'
      elif k == 'logy':
         if v: yaxis += 'l'
      elif k == 'majortickx':
         if v: xaxis += 't'
      elif k == 'majorticky':
         if v: yaxis += 't'
      elif k == 'number':
         for side in v:
            if   side == 'bottom':  xaxis += 'n'
            elif side == 'top':     xaxis += 'm'
            elif side == 'left':    yaxis += 'n'
            elif side == 'right':   yaxis += 'm'
            else: _error('_translateaxis(): unknown side for number: %s' %side)
      elif k == 'subtickx':
         if v: xaxis += 's'
      elif k == 'subticky':
         if v: yaxis += 's'
      elif k == 'tickstyle':
         if len(v) != 2: _error('_translateaxis(): drawtick must have two values!')
         if   v[0] == 'inside':     pass # the default
         elif v[0] == 'outside':    xaxis += 'i'
         elif v[0] == 'both':       xaxis += 'p'
         else: _error('_translateaxis(): unknown tickstyle location: %s' %v[0])

         if   v[1] == 'inside':     pass # the default
         elif v[1] == 'outside':    yaxis += 'i'
         elif v[1] == 'both':       yaxis += 'p'
         else: _error('_translateaxis(): unknown tickstyle location: %s' %v[1])
      elif k == 'verticaly':
         if v: yaxis += 'v'
      elif k == 'xinterval': pass
      elif k == 'yinterval': pass
      elif k == 'zerox':
         if not v: xaxis += 'o'
      elif k == 'zeroy':
         if not v: yaxis += 'o'
   if xaxis == '': xaxis = '0'
   if yaxis == '': yaxis = '0'
   return xaxis,yaxis

def _translatecolor(col):
   '''Take useful color string and convert to wip format.

      Note that for k and w, I assume you have changed your PGPLOT_BACKGROUND
      and PGPLOT_FOREGROUND colors so that black and white are switched.'''
   try:
      return str(list(_colors).index(col))
   except ValueError:
      _error('_translatecolor(): Invalid color name %s' %col)

def _translatecoords(text,coord):
   '''Translate ra/dec coordinates into ones useful for WIP'''
   if isinstance(text,str): # if a string, assume we have ra/dec coords
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
   '''Take useful fill string and convert to wip format.

      This is the type of fill string used for boxes.  For filled symbols,
      see below.'''
   try:
      return str(list(_fills).index(fill)+1)
   except ValueError:
      _error('_translatefill(): Invalid fill style %s.  Try s,h,/, or #.' %fillstr)

def _translatefillsymbol(style):
   '''Translate a symbol style into a fill style (later retranslated by
      _translatesymbol).  This is for filled symbols.  For filling of boxes, see
      above'''
   if   style == 'o':  fillstyle = 'fo'
   elif style == '^':  fillstyle = 'f^'
   elif style == 's':  fillstyle = 'fs'
   elif style == 'st': fillstyle = 'fst'
   else: _error('_translatefillsymbol(): Only circles, triangles, squares, and five-point stars can have a fill color!')

   if fillstyle in ['fo','fs']:
      fillfactor = 1.4
   elif fillstyle == 'fst':
      fillfactor = 0.8
   else:
      fillfactor = 0.8

   return fillstyle,fillfactor

def _translatefont(fontname):
   '''Translate a useful font name into wip.'''
   try:
      return str(list(_fonts).index(fontname)+1)
   except ValueError:
      _error('_translatefont(): Invalid font %s.  Try rm, it, sf, or cu!' %fontname)

def _translatelatex(latex):
   '''Translate latex string into something usable by WIP.'''

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

   if _optionsobj.font   == '1':
      defaultfont = r'\fn'
   elif _optionsobj.font == '2':
      defaultfont = r'\fr'
   elif _optionsobj.font == '3':
      defaultfont = r'\fi'
   elif _optionsobj.font == '4':
      defaultfont = r'\fs'
   else:
      _error('_translatelatex(): Invalid default font: %s!' %defaultfont)

   outstr = latex
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
   outstr = outstr.replace(r'#',r'\(733)') #wip thinks pound signs are comments
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

def _translatelevels(levels,unit):
   '''Translate levels which can be a list, tuple, string, or int into something
      usable by wip'''

   if isinstance(levels,str):
      levs = []
      blah = levels.split(':')
      try:
         blah2 = map(float,blah) # convert to floats
      except ValueError:
         _error('_translatelevels(): Specify levels values as numbers!')
      if len(blah2) == 3:
         if unit == 'step':
            count = int((blah2[1] - blah2[0])/blah2[2])
         elif unit == 'nbin':
            count = blah2[2]
            blah2[2] = (blah2[1] - blah2[0])/blah2[2] # set stepsize
         else:
            count = 39
            blah2[2] = blah2[1]
         if count > 39:
            _error('_translatelevels(): You cannot plot more than 40 contours!')
         elif count < 0:
            _error('_translatelevels(): Number of contour levels is negative!')
         levs = tuple(blah2[0] + n*blah2[2] for n in range(count+1))
      else:
         _error('_translatelevels(): Specify levels as val1:val2:val3 !')
      return ' '.join(map(str,levs))
   elif _isseq(levels):
      if len(levels) > 40:
         _error('_translatelevels(): You cannot plot more than 40 contours!')
      return ' '.join(map(str,levels))
   elif isinstance(levels,int):
      return levels
   else:
      _error('_translatelevels(): You must give a list/tuple, string, or integer for the levels command!')

def _translatepalette(palette):
   '''Translate a useful palette string to wip format.'''

   palettestr = str(palette)
   negFlag = 1 # set to one if we want the reverse palette
   if palettestr[0] == '-':
      negFlag = -1
      palettestr = palettestr[1:]
   try:
      return str(negFlag*(list(_palettes).index(palettestr)+1))
   except ValueError:
      _error('_translatepalette(): Invalid palette %s!' %palettestr)

def _translatelstyle(lstyle):
   '''Translate a useful line style into wip.'''
   try:
      return str(list(_lstyles).index(lstyle)+1)
   except ValueError:
      _error('_translatelstyle(): Invalid line style %s.  Try - , -- , .- , : , or -...' %lstyle)

def _translatesymbol(sym):
   '''Take useful symbol string and convert to wip format.'''
   symbol = str(sym) # ensure we have a string
   if   symbol == 's':     return '0'  # square
   elif symbol == '.':     return '1'  # dot
   elif symbol == '+':     return '2'  # plus sign
   elif symbol == '*':     return '3'  # asterisks
   elif symbol == 'o':     return '4'  # circle
   elif symbol == 'x':     return '5'  # cross
   elif symbol == '^':     return '7'  # triangle
   elif symbol == 'oplus': return '8'  # circle with plus sign
   elif symbol == 'odot':  return '9'  # circle with dot
   elif symbol == 'ps':    return '10' # pointed square
   elif symbol == 'd':     return '11' # diamond
   elif symbol == 'st':    return '12' # five-point star
   elif symbol == 'f^':    return '13' # filled triangle
   elif symbol == 'o+':    return '14' # open plus symbol
   elif symbol == 'david': return '15' # star of david
   elif symbol == 'fs':    return '16' # filled square
   elif symbol == 'fo':    return '17' # filled circle
   elif symbol == 'fst':   return '18' # filled five-point star
   else: return '99'

def _vptoxy(fp,x,y,r1,r2):
   '''Convert viewport x/y to physical x/y.

      x/y   - floats of x/y viewport values
      r1,r2 - strings of register names to set holding values'''
   fp.write(r'set %s ((x2 - x1) * (%s - vx1) / (vx2 - vx1)) + x1' %(r1,x))
   fp.write('\n')
   fp.write(r'set %s ((y2 - y1) * (%s - vy1) / (vy2 - vy1)) + y1' %(r2,y))
   fp.write('\n')

def _warning(msg):
   '''Print the warning message to standard error.'''
   if msg[-1] == '\n':
      sys.stderr.write('### PyWip Warning! %s' %msg)
   else:
      sys.stderr.write('### PyWip Warning! %s\n' %msg)

def _wipopen(funcname):
   '''Open the wip file for writing.  If one does not already exist, start a
      new one'''

   global _wipfile,_optionsobj,_panelobj

   if _wipfile == '???':
      tempfile.tempdir = os.getcwd()
      _wipfile = tempfile.mktemp(suffix='.wip')
      _optionsobj = _options()
      _panelobj = _panel()
      fp = open(_wipfile,'w')
      fp.write('set print ignore\n')
      fp.write('set maxarray 1000000\n') #TODO: does this work?
      fp.write('color %s\n'  %_optionsobj.color)
      fp.write('font %s\n'   %_optionsobj.font)
      fp.write('expand %s\n' %_optionsobj.size)
      fp.write('lstyle %s\n' %_optionsobj.lstyle)
      fp.write('lwidth %s\n' %_optionsobj.lwidth)
      fp.write('bgci %s\n'   %_optionsobj.bg)
      fp.write('### Start %s()\n' %funcname)
   else:
      fp = open(_wipfile,'a')
      fp.write('### Start %s()\n' %funcname)
   return fp

def _xytovp(fp,x,y,r1,r2):
   '''Convert x/y values to viewport values
      x,y   - x and y coordinates as floats
      r1,r2 - strings of registers or variables to set holding values'''
   fp.write(r'set %s ((vx2 - vx1) * (%s - x1) / (x2 - x1)) + vx1' %(r1,x))
   fp.write('\n')
   fp.write(r'set %s ((vy2 - vy1) * (%s - y1) / (y2 - y1)) + vy1' %(r2,y))
   fp.write('\n')
