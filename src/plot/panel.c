/*
	<panel.c>
	02mar91 jm  Original code.
	16jul91 jm  Major overhaul.
	24jun96 jm  Made negative third argument mean left-right-top-down.

Routines:
void wippanel ARGS(( int nx, int ny, int k ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Sets the viewport so that there are NX panels across and
 *  NY panels up and down.  NX = NY = 1 resets to the state to
 *  the viewport state that existed prior to PANEL being called.
 *
 *  A positive K argument means that the counting starts at the
 *  lower left box and increases to the right and then upward.
 *  If K is negative, then the counting begins in the upper left
 *  box and increases to the right and then downward.
 *
 *  If NX and/or NY < 0, then there will be no gap between the panels.
 */
#ifdef PROTOTYPE
void wippanel(int nx, int ny, int k)
#else
void wippanel(nx, ny, k)
int nx, ny, k;
#endif /* PROTOTYPE */
{
    int indx, indy;
    float deltax, deltay, offx, offy;
    float vx1, vx2, vy1, vy2;
    float xmarg, ymarg;
    float left, right, bottom, top;
    double chsiz;
    static int oldnx = 0;
    static int oldny = 0;
    static float oldvx1, oldvx2, oldvy1, oldvy2;
    static double oldsiz;
    LOGICAL error, reset, touchx, touchy;

    /* First return if dangerous (incorrect) inputs are given. */

    if ((nx == 0) || (ny == 0)) return;

    reset = FALSE;
    if ((nx != oldnx) || (ny != oldny)) reset = TRUE;

    /* PANEL 1 1 K or RESET == TRUE resets location to stored values. */

    if (((nx == 1) && (ny == 1)) || (reset == TRUE)) {
      if ((oldnx != 0) && (oldny != 0)) {
        wipexpand(oldsiz);
        cpgsvp(oldvx1, oldvx2, oldvy1, oldvy2);
        wipviewport();
      }
      oldnx = 0;
      oldny = 0;
      if ((nx == 1) && (ny == 1)) return;
    }
    oldnx = nx;
    oldny = ny;

    /* Get some parameters needed for this routine. */

    chsiz = wipgetvar("expand", &error);
    if (error == TRUE) chsiz = 1.0;
    wipgetvp(&vx1, &vx2, &vy1, &vy2);
    wipgetsubmar(&xmarg, &ymarg);

    /*
     * If nx/ny are negative or either is equal to 1, set a variable so
     * that adjacent sides touch (or equals the full viewport).
     */

    touchx = (nx < 2) ? TRUE : FALSE ;
    touchy = (ny < 2) ? TRUE : FALSE ;
    nx = ABS(nx);
    ny = ABS(ny);

    /* PANEL M N K means save the old location values and set new ones. */

    if (((nx != 1) || (ny != 1)) && (reset == TRUE)) {
      oldvx1 = vx1;
      oldvx2 = vx2;
      oldvy1 = vy1;
      oldvy2 = vy2;
      oldsiz = chsiz;
    }

    if (touchx == TRUE) {
      left = oldvx1;
      right = oldvx2;
    } else {
      left = oldvx1 - (0.025 * xmarg * oldsiz);
      right = oldvx2 + (0.025 * xmarg * oldsiz);
    }

    if (touchy == TRUE) {
      bottom = oldvy1;
      top = oldvy2;
    } else {
      bottom = oldvy1 - (0.025 * ymarg * oldsiz);
      top = oldvy2 + (0.025 * ymarg * oldsiz);
    }

    chsiz = oldsiz * POW((1.0/nx/ny), 0.3333);

    deltax = (right - left) / nx;
    if (touchx == TRUE) {
      offx = 0.0;
    } else {
      offx = 0.025 * xmarg * chsiz;
    }

    deltay = (top - bottom) / ny;
    if (touchy == TRUE) {
      offy = 0.0;
    } else {
      offy = 0.025 * ymarg * chsiz;
    }

    if (k > 0) {
      indx = (k - 1) % nx;
      indy = (k - 1) / nx;
    } else if (k < 0) {
      indx = (-k - 1) % nx;
      indy = ny - 1 - ((-k - 1) / nx);
    } else {
      indx = 0;
      indy = 0;
    }

    vx1 = left + offx + (indx * deltax);
    vx2 = vx1 + deltax - (2.0 * offx);
    vy1 = bottom + offy + (indy * deltay);
    vy2 = vy1 + deltay - (2.0 * offy);
    wipexpand(chsiz);
    cpgsvp(vx1, vx2, vy1, vy2);
    wipviewport();

    return;
}
