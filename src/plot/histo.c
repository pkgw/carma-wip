/*
        <histo.c>
        02nov89 mchw Original Fortran code.
        14aug92 jm   Modified code to C and WIP.
        15aug92 jm   Added wipbar() code (originally written 11/88 jm).
        24aug94 jm   Modified wipbar() to optionally specify the "bottom"
		     of the bar and to also specify the "width" of a bar.

Routines:
int wiphline ARGS((int npts, float x[], float y[], float gap, int center));
int wipbar ARGS((int nxy, float x[], float y[], int nc, float col[], int loc,
  int dolimit, float barlimit, float barwidth));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Histogram style line plot of Y array versus X array.  Points
 *  are not connected over gaps or reversals in the X array.
 *  "gap" is the factor to define where a gap occurs in X array
 *  (ie. a gap is defined TRUE when (x[i+1]-x[i]) > gap*(x[i]-x[i-1])).
 *  if "center" is non-zero, the X values denote the center of the
 *  bin; otherwise, the X values denote the lower left edge (in X)
 *  of the bin.
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wiphline(int npts, float x[], float y[], float gap, int center)
#else
int wiphline(npts, x, y, gap, center)
int npts;
float x[], y[];
float gap;
int center;
#endif /* PROTOTYPE */
{
    register int i, start, end;
    float xp, xm, yl;
    LOGICAL atgap, reverse;

    if (npts < 2) {
      wipoutput(stderr, "Insufficient number of points to bin.\n");
      return(1);
    }

    cpgbbuf(); /* Set up buffered output. */

/*  Look for gaps or reversals in x-array. */

    start = 0;
    end = 2;
    while (end <= npts) {
      if ((npts > 2) && (end < npts)) {
        xp = x[end] - x[end-1];
        xm = x[end-1] - x[end-2];
        atgap = (ABS(xp) > ABS(gap * xm)) ? TRUE : FALSE;
        reverse = ((xm * xp) < 0) ? TRUE : FALSE;
      } else {
        atgap = TRUE;
      }

/*  Connect sequences of points between gaps and reversals. */

      if ((atgap == TRUE) || (reverse == TRUE)) {
        if (center) {
          wipmove(x[start], y[start]);
          for (i = start; i < (end-1); i++) {
            xm = 0.5 * (x[i] + x[i + 1]);
            wipdraw(xm, y[i]);
            wipdraw(xm, y[i + 1]);
          }
          wipdraw(x[end - 1], y[end - 1]);
        } else {
          wipmove(x[start], (yl = y[start]));
          for (i = start + 1; i < end; i++) {
            wipdraw(x[i], yl);
            wipdraw(x[i], (yl = y[i]));
          }
        }
        start = end + 1;
        end += 2;
      } else {
        end++;
      }
    }

    cpgebuf(); /* Finish up buffered output. */
    return(0);
}

/*
 *  Draws a bar graph in the direction specified by "location".
 *  If "location" is 1, bar extends in +X direction; 2, +Y; 3, -X; 4, -Y.
 *  The color[] array (size "nc") specifies the color to draw each bar.
 *  If "npts" is greater than "nc", then color[0] is used for all bar
 *  plots beyond "nc-1".  If "nc" is zero, then the current color will
 *  be used for all bar plots.  The current line and fill styles control
 *  the type of plot generated.  If dolimit is non-zero, then the value
 *  of barlimit is used to demark the "bottom" of the bar.
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wipbar(int npts, float x[], float y[], int nc, float color[], int location, int dolimit, float barlimit, float barwidth)
#else
int wipbar(npts, x, y, nc, color, location, dolimit, barlimit, barwidth)
int npts;
float x[], y[];
int nc;
float color[];
int location;
int dolimit;
float barlimit;
float barwidth;
#endif /* PROTOTYPE */
{
    register int j, ic, ibar;
    int cmin, cmax, newColor;
    int saveColor, lastColor, defaultColor;
    float halfwidth;
    float x1, x2, y1, y2;
    float xleft, xrite, ybot, ytop;
    float xlast, ylast, xave, yave;
    double arg;
    LOGICAL error;

    if ((barwidth > 0.0) && (npts < 1))
      return(1);
    else if ((barwidth <= 0) && (npts < 2))
      return(1);

    arg = wipgetvar("color", &error);
    saveColor = (error == TRUE) ? 1 : NINT(arg) ;
    cpgqcol(&cmin, &cmax);

    lastColor = defaultColor = saveColor;
    if ((nc > 0) && (color[0] >= cmin) && (color[0] <= cmax))
      defaultColor = color[0];

    ibar = (location - 1) % 4;

    wipgetlimits(&xleft, &xrite, &ybot, &ytop);
    if (dolimit != 0) {
      switch (ibar) {
        case 0: xleft = barlimit; break;
        case 1: ybot  = barlimit; break;
        case 2: xrite = barlimit; break;
        case 3: ytop  = barlimit; break;
      }
    }

    cpgbbuf(); /* Set up buffered output. */

    if (barwidth > 0.0) {
      halfwidth = barwidth / 2.0;
      for (j = 0; j < npts; j++) {
        x1 = x[j] - halfwidth;  x2 = x[j] + halfwidth;
        y1 = y[j] - halfwidth;  y2 = y[j] + halfwidth;

        switch (ibar) {
          case 0: x1 = xleft;  x2 = x[j];   break;
          case 1: y1 = ybot;   y2 = y[j];   break;
          case 2: x1 = x[j];   x2 = xrite;  break;
          case 3: y1 = y[j];   y2 = ytop;   break;
        }

        ic = j;
        if ((nc > 0) && (ic < nc) && (color[ic] >= cmin) && (color[ic] <= cmax))
          newColor = color[ic];
        else
          newColor = defaultColor;

        if (newColor != lastColor)
          wipcolor(newColor);
        lastColor = newColor;

        cpgrect(x1, x2, y1, y2);
      }
    } else {
      xlast = 0.5 * ((3.0 * x[0]) - x[1]);
      ylast = 0.5 * ((3.0 * y[0]) - y[1]);
      for (j = 1; j <= npts; j++) {
        if (j < npts) {
          xave = 0.5 * (x[j] + x[j - 1]);
          yave = 0.5 * (y[j] + y[j - 1]);
        } else {
          xave = 0.5 * ((3.0 * x[npts - 1]) - x[npts - 2]);
          yave = 0.5 * ((3.0 * y[npts - 1]) - y[npts - 2]);
        }

        switch (ibar) {
          case 0: x1 = xleft;  x2 = x[j-1]; y1 = ylast;  y2 = yave;   break;
          case 1: x1 = xlast;  x2 = xave;   y1 = ybot;   y2 = y[j-1]; break;
          case 2: x1 = x[j-1]; x2 = xrite;  y1 = ylast;  y2 = yave;   break;
          case 3: x1 = xlast;  x2 = xave;   y1 = y[j-1]; y2 = ytop;   break;
        }

        ic = j - 1;
        if ((nc > 0) && (ic < nc) && (color[ic] >= cmin) && (color[ic] <= cmax))
          newColor = color[ic];
        else
          newColor = defaultColor;

        if (newColor != lastColor)
          wipcolor(newColor);
        lastColor = newColor;

        cpgrect(x1, x2, y1, y2);
        xlast = xave; ylast = yave;
      }
    }

    if (saveColor != lastColor)
      wipcolor(saveColor);

    cpgebuf(); /* Finish up buffered output. */

    return(0);
}
