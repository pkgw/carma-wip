/*
	<arc.c>
	17jun85 jm  Original Code.
	??feb86 jm  Arcangle parameter added.
	??sep86 jm  Start parameter added and code converted to
		    calculate radius.
	07oct87 jm  Standardized Fortran.
	04oct90 jm  Converted to C.
	01apr91 jm  Angle parameter added.
	27apr91 jm  Changed inputs from semi-major to major axes.
	10jun91 jm  Heavily modified to: 1) calculate unrotated ellipse
		    values; 2) rotate to new coordinates by value of ANGLE.
		    Also, used malloc to get work array storage.
	19jul91 jm  Modified to do all work in viewport coordinates and
		    transform back at the last minute.
	06aug91 jm  Removed 19jul91 modification and added a fix to
		    insure that angle always increases counter-clockwise.
	15aug92 jm  Modified to return int status to indicate success.
	28sep92 jm  Modified to reduce the number of calls to sin/cos.
		    Also modified algorithm to no longer need sqrt function.
	12oct95 jm  Added beam command.
	24apr96 jm  Fixed arc/beam commands so they work properly when
		    an axis direction is reversed (eg. RA).
	02oct96 jm  Added offx/y for beam command.

Routines:
int wiparc ARGS(( float majx, float majy, float arcangle, float angle,
  float start ));
int wipbeam ARGS(( float major, float minor, float posangle, float offx,
  float offy, int fillcolor, float scale, int bgrect ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  This routine will draw an ellipse for a given major axis in two
 *  perpendicular directions (MAJX and MAJY).  ANGLE is the rotation
 *  of the semi-major axis.  ARCANGLE determines the angular extent
 *  of the arc and START sets the initial offset (relative to the
 *  major axis) of the beginning of the ellipse.  All angles are in
 *  units of degrees with zero defined in the direction of the
 *  POSITIVE x-axis and increasing towards the POSITIVE y-axis
 *  (regardless of whether or not the X/Y World coordinates increase
 *  that way).
 *
 *  Algorithm assumes the following equations hold:
 *
 *    x'**2     y'**2
 *    -----  +  -----  = 1;   x' = a * cos(alpha); y' = b * sin(alpha).
 *    a**2      b**2
 *
 *    (x - xc) = (x' * cos(angle)) - (y' * sin(angle))
 *    (y - yc) = (x' * sin(angle)) + (y' * cos(angle))
 *
 *  where a is the semi-major axis in the x'-direction, b is the
 *  semi-major axis in the y'-direction, alpha is an angle (measured
 *  positive as defined above), and (x',y') is a position on the ellipse
 *  as viewed in the rotated frame.  The final ellipse is rotated back
 *  and offset from the origin to the center position (xc,yc).
 *
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wiparc(float majx, float majy, float arcangle, float angle, float start)
#else
int wiparc(majx, majy, arcangle, angle, start)
float majx, majy, arcangle, angle, start;
#endif /* PROTOTYPE */
{
    register int j;
    int nstep;
    float cx, cy;
    float oldcx, oldcy;
    float expand;
    float x1, x2, y1, y2;
    float oldx1, oldx2, oldy1, oldy2;
    float a, b, theta, alpha;
    float costheta, sintheta, cosalpha, sinalpha;
    float savecos, cosstep, sinstep;
    float step, division;
    float delx, dely;
    float *xarc, *yarc;
    double arg;
    LOGICAL error;

    if ((majx <= 0.0) || (majy <= 0.0)) {
      wipoutput(stderr, "Both MAJX/Y must be > 0: %f %f\n", majx, majy);
      return(1);
    }

    /* Find the cursor position and current character expansion. */
    wipgetlimits(&oldx1, &oldx2, &oldy1, &oldy2);
    wipgetcxy(&oldcx, &oldcy);
    arg = wipgetvar("expand", &error);
    expand = (error == TRUE) ? 1.0 : arg ;

    a = majx / 2.0;
    b = majy / 2.0;

    alpha = start * RPDEG;
    theta = angle * RPDEG;
    costheta = COS(theta);
    sintheta = SIN(theta);

    step = arcangle;
    if (ABS(arcangle) > 360.0) step -= (360.0 * INT(arcangle / 360.0));
    division = 360.0 * expand;
    step *= (RPDEG / division);
    nstep = NINT(division + 1.0);

    xarc = vector(nstep);
    yarc = vector(nstep);
    if ((xarc == (float *)NULL) || (yarc == (float *)NULL)) {
      wipoutput(stderr, "ARC: Cannot allocate internal work arrays.\n");
      wipoutput(stderr, "ARC: Please reduce the value of EXPAND.\n");
      if (xarc != (float *)NULL) freevector(xarc);
      if (yarc != (float *)NULL) freevector(yarc);
      return(1);
    }
    /*
     *  To make sure the angle of the arc is clockwise always (even
     *  when the axis direction is reversed), reset the limits.
     */
    x1 = 0.0;
    x2 = ABS(oldx2 - oldx1);
    y1 = 0.0;
    y2 = ABS(oldy2 - oldy1);
    cx = (oldcx - oldx1) * x2 / (oldx2 - oldx1);
    cy = (oldcy - oldy1) * y2 / (oldy2 - oldy1);
    cpgswin(x1, x2, y1, y2);
    wiplimits();
    wipmove(cx, cy);

    /*
     *  To reduce the number of calls to sin and cos, use the identities:
     *  cos(alpha+step) = cos(alpha)cos(step) - sin(alpha)sin(step)
     *  sin(alpha+step) = sin(alpha)cos(step) + cos(alpha)sin(step)
     *  This lets the angle "alpha" to be incremented by an amount
     *  "step" and the new cos(alpha) & sin(alpha) to be computed
     *  without having to call cos() or sin() again.
     */
    sinstep = SIN(step);
    cosstep = COS(step);
    sinalpha = SIN(alpha);
    cosalpha = COS(alpha);

    for (j = 0; j < nstep; j++) {                  /* Fill the arrays. */
      delx = a * cosalpha;
      dely = b * sinalpha;
      /* Rotate from prime coordinates to viewport (x, y). */
      xarc[j] = cx + ((delx * costheta) - (dely * sintheta));
      yarc[j] = cy + ((delx * sintheta) + (dely * costheta));
      /*  Increment "alpha" by an amount of "step". */
      savecos = (cosalpha * cosstep) - (sinalpha * sinstep);
      sinalpha = (sinalpha * cosstep) + (cosalpha * sinstep);
      cosalpha = savecos;
    }

    arg = wipgetvar("fill", &error);
    if ((error == TRUE) || (NINT(arg) == 2)) {
      cpgline(nstep, xarc, yarc); /* This avoids the closing of the arc. */
    } else {
      cpgpoly(nstep, xarc, yarc);
    }

    /* Reset the original coordinate system. */
    cpgswin(oldx1, oldx2, oldy1, oldy2);
    wiplimits();
    wipmove(oldcx, oldcy);

    freevector(xarc);              /* Release the local array storage. */
    freevector(yarc);
    return(0);
}

/*
 *  This routine draws a beam centered on the current cursor position
 *  using the current color and line width.  The actual center of the
 *  beam is offset by offx/y beam width/heights.  If scale is positive,
 *  then that scaling factor will be used on the X-axis; otherwise a
 *  scaling of 15*cos(Dec) will be used.  The latter should be used
 *  when the axes reflect RA-Dec.  Use scale=1 if no scaling of the
 *  X-axis is done.  The major and minor axes of the beam should be in
 *  the same units as the two axes.  This means if scale is not one,
 *  then they should both be in the same units as the Y-axis.  If
 *  bgrect is not negative, then a solid rectangle will be drawn in
 *  the bgrect color first.  The rectangle will be as large as the beam
 *  extent.  The beam will first be drawn solid using the input
 *  fillcolor and then hollow with the current color.
 *
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wipbeam(float major, float minor, float posangle, float offx,
    float offy, int fillcolor, float scale, int bgrect)
#else
int wipbeam(major, minor, posangle, offx, offy, fillcolor, scale, bgrect)
float major;
float minor;
float posangle;
float offx;
float offy;
int fillcolor;
float scale;
int bgrect;
#endif /* PROTOTYPE */
{
    int color, fill, style;
    int status;
    float sx, sy, sp, cp;
    float x1, x2, y1, y2, cx, cy;
    float oldx1, oldx2, oldy1, oldy2; 
    float rectx1, rectx2, recty1, recty2; 
    float oldcx, oldcy;
    double darg, factor;
    LOGICAL error;

    wipgetlimits(&oldx1, &oldx2, &oldy1, &oldy2);
    wipgetcxy(&oldcx, &oldcy);
    darg = wipgetvar("color", &error);
    color = (error == TRUE) ? 1 : NINT(darg);
    darg = wipgetvar("fill", &error);
    fill = (error == TRUE) ? 1 : NINT(darg);
    darg = wipgetvar("lstyle", &error);
    style = (error == TRUE) ? 1 : NINT(darg);

    /* Determine the extent in the X and Y directions of the beam. */
    sp = SIN(RPDEG * (90.0 + posangle));
    cp = COS(RPDEG * (90.0 + posangle));
    x1 = major * cp;
    x2 = minor * sp;
    y1 = major * sp;
    y2 = minor * cp;
    sx = SQRT((x1 * x1) + (x2 * x2));
    sy = SQRT((y1 * y1) + (y2 * y2));

    if (scale > 0) {
      factor = scale;
    } else {
      factor = 15.0 * COS(RPDEG * (oldy1 + oldy2) / (2.0 * 3600.0));
    }
    x1 = 0.0;
    x2 = (oldx2 - oldx1) * factor;
    y1 = 0.0;
    y2 = oldy2 - oldy1;
    cx = (oldcx - oldx1) * factor;
    cy = oldcy - oldy1;
    cpgswin(x1, x2, y1, y2);
    wiplimits();

    cx += (sx * offx * ((x2 < 0) ? -1 : 1));
    cy += (sy * offy * ((y2 < 0) ? -1 : 1));

    wipmove(cx, cy);
    wipfill(1);
    wipltype(1);

    if (bgrect >= 0) {
      rectx1 = cx - (sx / 2.0);
      rectx2 = rectx1 + sx;
      recty1 = cy - (sy / 2.0);
      recty2 = recty1 + sy;
      wipcolor(bgrect);
      cpgrect(rectx1, rectx2, recty1, recty2);
    }

    wipcolor(fillcolor);
    status = wiparc(major, minor, 360.0, (90.0 + posangle), 0.0);

    wipcolor(color);
    if (status == 0) {
      wipfill(2);
      status = wiparc(major, minor, 360.0, (90.0 + posangle), 0.0);
    }
    
    wipfill(fill);
    wipltype(style);
    cpgswin(oldx1, oldx2, oldy1, oldy2);
    wiplimits();
    wipmove(oldcx, oldcy);

    return(status);
}
