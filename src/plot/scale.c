/*
	<scale.c>
	12mar91 jm  Original code.
        02aug92 jm  Modified to return status (void -> int) rather than
		    using a passed LOGICAL pointer.

Routines:
int wipscale ARGS(( float scalex, float scaley, int k ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Sets the viewport scale so that there are x-mm across and y-mm
 *  up and down.  The input scale factors determine the number of
 *  world coordinate units per mm in each direction.  This may allow
 *  a viewport to be generated larger than the view surface...
 *  Hey, user beware!
 *
 *  K is an integer that specifies the type of conversion the two scale
 *  terms will be performing.  If K is 0, the scale terms are dimensioned
 *  as world (user) units per normalized device coordinate; K is 1, means
 *  world per inch; K is 2, world per mm; and K is 3, world per pixel.
 *  If K is not one of the accepted values, no conversion is performed.
 *  Also, if either SCALEX or SCALEY are less than or equal to 0, no
 *  conversion is performed.
 *
 *  Returns 0 if success; 1 on error.
 */
#ifdef PROTOTYPE
int wipscale(float scalex, float scaley, int k)
#else
int wipscale(scalex, scaley, k)
float scalex, scaley;
int k;
#endif /* PROTOTYPE */
{
    float deltax, deltay;
    float x1, x2, y1, y2;
    float vx1, vx2, vy1, vy2;
    float vix1, vix2, viy1, viy2;

    /* First return if dangerous (incorrect) inputs are given. */
    if ((scalex <= 0) || (scaley <= 0)) {
      wipoutput(stderr, "Negative or zero scale values are not permitted.\n");
      return(1);
    }

    if ((k < 0) || (k > 3)) {
      wipoutput(stderr, "Conversion type unknown: [%d].\n", k);
      return(1);
    }

    wipgetlimits(&x1, &x2, &y1, &y2);   /* Get world coordinate range. */
    deltax = ABS(x2 - x1) / scalex;
    deltay = ABS(y2 - y1) / scaley;

                                    /* Return viewport size in inches. */
    cpgqvp(1, &vix1, &vix2, &viy1, &viy2);

    if (k != 1) {              /* Conversion not being done in inches. */
      cpgqvp(k, &vx1, &vx2, &vy1, &vy2);  /* VP size in desired units. */
      deltax *= ((vix2 - vix1) / (vx2 - vx1));
      deltay *= ((viy2 - viy1) / (vy2 - vy1));
    }

    vx1 = vix1;
    vx2 = vix1 + deltax;
    vy1 = viy1;
    vy2 = viy1 + deltay;

    cpgvsiz(vx1, vx2, vy1, vy2);           /* Set the scaled viewport. */
    wipviewport();         /* Make sure WIP's internal values are set. */

    return(0);
}
