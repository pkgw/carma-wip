/*
	<arrow.c>
	05nov93 jm  Original code.
	12oct95 jm  Modified wiparrow to call wipgetcxy.

Routines:
void wiparrow ARGS(( float xp, float yp, float angle, float vent ));
void wipvfield ARGS(( float x[], float y[], float r[], float phi[], int npts, float angle, float vent ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Arrow draws one arrow from the current cursor position to the
 *  specified position (xp, yp).  The acute angle of the arrow point,
 *  in degrees, is specified by the input "angle"; angles in the
 *  range 20.0 to 90.0 give reasonable results.  The default angle
 *  is 45.0 degrees.  The fraction of the triangular arrow-head that
 *  is cut away from the back is specified by the input "vent".  A
 *  value of 0.0 gives a triangular wedge arrow-head; 1.0 gives an
 *  open >.  Values of 0.3 to 0.7 give reasonable results.  The
 *  default value for "vent" is 0.3.
 */
#ifdef PROTOTYPE
void wiparrow(float xp, float yp, float angle, float vent)
#else
void wiparrow(xp, yp, angle, vent)
float xp;
float yp;
float angle;
float vent;
#endif /* PROTOTYPE */
{
    int fill;
    float cx, cy;
    double darg;
    LOGICAL error;

    wipgetcxy(&cx, &cy);
    darg = wipgetvar("fill", &error);
    fill = (error == TRUE) ? 1 : NINT(darg);

    cpgsah(fill, angle, vent);
    cpgarro(cx, cy, xp, yp);

    return;
}

#ifdef PROTOTYPE
void wipvfield(float x[], float y[], float r[], float phi[], int npts, float angle, float vent)
#else
void wipvfield(x, y, r, phi, npts, angle, vent)
float x[];
float y[];
float r[];
float phi[];
int npts;
float angle;
float vent;
#endif /* PROTOTYPE */
{
    register int i;
    int fill;
    float x1, y1, x2, y2;
    double darg;
    LOGICAL error;

    darg = wipgetvar("fill", &error);
    fill = (error == TRUE) ? 1 : NINT(darg);

    cpgbbuf();
    cpgsah(fill, angle, vent);

    for (i = 0; i < npts; i++) {
      x1 = x[i];
      y1 = y[i];
      x2 = x1 + (r[i] * COS(phi[i] * RPDEG));
      y2 = y1 + (r[i] * SIN(phi[i] * RPDEG));
      cpgarro(x1, y1, x2, y2);
    }

    cpgebuf();

    return;
}
