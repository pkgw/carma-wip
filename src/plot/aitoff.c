/*
	<aitoff.c>
	??jul87 jm  Original Fortan code.
	03aug92 jm  Converted to C code.
	27aug92 jm  Fixed a bug and moved (file static) global
		    variables into the call statement.
	18jul94 jm  Globe was flipped in x/y; fixed now.

Routines:
static void setaitoff ARGS(( float *xscale, float *yscale, float *xorg, float *yorg ));
static void aitoffConvert ARGS(( float l, float b, float *x, float *y ));
void wipaitoff ARGS(( int nxy, float x[], float y[] ));
void wipaitoffgrid ARGS(( int nlong, int nlats ));
*/

#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE is defined in wip.h. */

/* Code */

/*
 *  This routine returns a pointer scale and offset parameters.
 *  These are used to convert from internal (Aitoff) units back
 *  to the User's world coordinates.
 *
 *  The Aitoff routines scale the current device (internally) so that
 *  a rectangle dimension 4 x 2 units can be plotted horizontally.
 *  This is the bounding rectangle of the Aitoff plot.
 */
#ifdef PROTOTYPE
static void setaitoff(float *xscale, float *yscale, float *xorg, float *yorg)
#else
static void setaitoff(xscale, yscale, xorg, yorg)
float *xscale, *yscale, *xorg, *yorg;
#endif /* PROTOTYPE */
{
    float rmax, scale;
    float xszdef, yszdef, xszusr, yszusr;
    float x1, x2, y1, y2;
    float vx1, vx2, vy1, vy2;

    wipgetlimits(&x1, &x2, &y1, &y2);
    wipgetvp(&vx1, &vx2, &vy1, &vy2);

    xszdef = ABS(vx2 - vx1);
    yszdef = ABS(vy2 - vy1);
    xszusr = ABS(x2 - x1);
    yszusr = ABS(y2 - y1);

    *xorg = (xszusr / 2.0) + x1;
    *yorg = (yszusr / 2.0) + y1;
    *xscale = xszusr / xszdef;
    *yscale = yszusr / yszdef;

    rmax = MAX((4.0 / xszdef), (2.0 / yszdef));
    scale = 1.0 / rmax;
    *xscale *= scale;
    *yscale *= scale;

    return;
}

/*
 *  This routine will convert longitude and latitude values ("l" and "b")
 *  into equivalent cartesian coordinates "x" and "y".  The return values
 *  will be in internal Aitoff units where "*x" is in the range [-2, 2]
 *  and "*y" is in the range [-1, 1].
 */
#ifdef PROTOTYPE
static void aitoffConvert(float l, float b, float *x, float *y)
#else
static void aitoffConvert(l, b, x, y)
float l, b;
float *x, *y;
#endif /* PROTOTYPE */
{
    float lover2, den;
    float radl, radb;

   /*
    *  Force "l" to be in the range (-180 <= l <= +180) and
    *  "b" to be in the range (-90 <= b <= +90).
    */

    while (l < -180.0) l += 360;
    while (l >  180.0) l -= 360;
    while (b <  -90.0) b += 180;
    while (b >   90.0) b -= 180;

    /*  Convert l and b to radians. */

    radl = l * RPDEG;
    radb = b * RPDEG;

    lover2 = radl / 2.0;
    den = SQRT(1.0 + (COS(radb) * COS(lover2)));
    *x = 2.0 * COS(radb) * SIN(lover2) / den;
    *y = SIN(radb) / den;

    /* "*x" is now in the range [-2, 2] and "*y" in the range [-1, 1]. */

    return;
}

/*  Converts elements in x and y arrays to Aitoff positions. */
#ifdef PROTOTYPE
void wipaitoff(int nxy, float x[], float y[])
#else
void wipaitoff(nxy, x, y)
int nxy;
float x[], y[];
#endif /* PROTOTYPE */
{
    register int j;
    float xout, yout;
    float xscale, yscale, xorigin, yorigin;

    setaitoff(&xscale, &yscale, &xorigin, &yorigin);

    for (j = 0; j < nxy; j++) {
      aitoffConvert(x[j], y[j], &xout, &yout);
      x[j] = (xout * xscale) + xorigin;
      y[j] = (yout * yscale) + yorigin;
    }

    return;
}

/*
 *  This routine generates an Aitoff equal-area projection of the
 *  whole sky, centered on (long = 0, lat = 0).
 */
#ifdef PROTOTYPE
void wipaitoffgrid(int nlong, int nlats)
#else
void wipaitoffgrid(nlong, nlats)
int nlong;
int nlats;
#endif /* PROTOTYPE */
{
    register int j;
    float bb, ll, xc, yc;
    float deglong, deglat;
    float xscale, yscale, xorigin, yorigin;

    /*
     *  Scale the device so that a rectangle dimension 2 x 4 units
     *  can be plotted horizontally.  This is the bounding rectangle
     *  of the Aitoff plot.
     */
    setaitoff(&xscale, &yscale, &xorigin, &yorigin);

    /*  Account for the edges: add 2 to both nlong and nlats. */
    nlong += 2;
    nlats += 2;

    if (nlong < 2) nlong = 2;
    deglong = 360.0 / (float)(nlong - 1);
    deglat = (nlats > 2) ? (180.0 / (float)(nlats - 1)) : 180.0;

    cpgbbuf();                               /* Start buffered output. */

    /*
     *  Draw "nlong" lines of constant longitude at longitude steps of
     *  "deglong" degrees.  Each line is made up of 180 straight-line
     *  segments.
     */
    for (j = 0; j < nlong; j++) {
      ll = -180.0 + (j * deglong);
      bb = -90;
      aitoffConvert(ll, bb, &xc, &yc);
      wipmove(((xc * xscale) + xorigin), ((yc * yscale) + yorigin));
      while (bb++ < 90) {
        aitoffConvert(ll, bb, &xc, &yc);
        wipdraw(((xc * xscale) + xorigin), ((yc * yscale) + yorigin));
      }
    }

    /*
     *  Draw "nlats" lines of constant latitude at latitude steps of
     *  "deglat" degrees.  Each line is made up of 360 straight-line
     *  segments.
     */
    for (j = 0; j < nlats; j++) {
      bb = -90.0 + (j * deglat);
      if (ABS(bb) > 85.0) continue;             /* Avoid overcrowding. */
      ll = -180;
      aitoffConvert(ll, bb, &xc, &yc);
      wipmove(((xc * xscale) + xorigin), ((yc * yscale) + yorigin));
      while (ll++ < 180) {
        aitoffConvert(ll, bb, &xc, &yc);
        wipdraw(((xc * xscale) + xorigin), ((yc * yscale) + yorigin));
      }
    }

    cpgebuf();                                /* Flush out the buffer. */
    return;
}
