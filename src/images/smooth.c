/*
	<smooth.c>
	10jul96 jm  Original Code.

Routines:
int wipsmooth ARGS(( float **array, int nx, int ny, int order, float blank ));
*/

#include "wip.h"

/* Global variables for just this file */

/*
 *  The parameter WTFAC is the value of sigma for a FWHM of 1 pixel
 *  (i.e. WTFAC = ((FWHM)**2) / (4 * ln(2)) with FWHM = 1).  This weight
 *  means that the nearest neighbor will only receive about 6.25% of
 *  the central component and the next nearest only 0.4%.
 */

#define WTFAC 0.36067376

/* Code */

/*
 *  This routine smooths the input array by applying a filter to each
 *  point and replacing it by the weighted mean.  Blank values are
 *  ignored in the smoothing.
 *
 *  Returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wipsmooth(float **array, int nx, int ny, int order, float blank)
#else
int wipsmooth(array, nx, ny, order, blank)
float **array;
int nx;
int ny;
int order;
float blank;
#endif /* PROTOTYPE */
{
    register int x, y, x1, x2, y1, y2, iy, jy, ix, mag;
    float xx, yy;
    float wt;
    float **sum, **weight;

    if ((nx < 1) || (nx < 1)) {
      wipoutput(stderr, "Image dimensions must be > 0: %dx%d\n", nx, ny);
      return(1);
    }

    if ((order < 0) || (order > 2)) {
      wipoutput(stderr, "Smoothing order should be [0..2].\n");
      return(1);
    }

    wipoutput(stdout, "Smoothing the image data (order %d).\n", order);

    if (order == 0)                       /* Null effect; return okay. */
      return(0);

    mag = (2 * order) + 1;
    if (mag > ny) mag = ny;

    sum = matrix(0, nx, 0, mag);
    weight = matrix(0, nx, 0, mag);
    if ((sum == (float **)NULL) || (weight == (float **)NULL)) {
      wipoutput(stderr,
        "Could not allocate internal work arrays for smoothing.\n");
      if (sum != (float **)NULL) freematrix(sum, 0, 0);
      if (weight != (float **)NULL) freematrix(weight, 0, 0);
      return(1);
    }

    for (y = 0; y < mag; y++) {         /* Initialize the work arrays. */
      for (x = 0; x < nx; x++) {
        sum[x][y] = 0.0;
        weight[x][y] = 0.0;
      }
    }

    for (y = 0; y < ny; y++) {
      y1 = y - order;
      y2 = y + order;
      if (y1 < 0) y1 = 0;
      if (y2 >= ny) y2 = ny - 1;

      for (x = 0; x < nx; x++) {
        if (array[y][x] == blank) continue;  /* Ignore blanked values. */

        x1 = x - order;
        x2 = x + order;
        if (x1 < 0) x1 = 0;
        if (x2 >= nx) x2 = nx - 1;

        for (iy = y1; iy <= y2; iy++) {
          jy = iy % mag;
          yy = (iy - y) * (iy - y);
          for (ix = x1; ix <= x2; ix++) {
            xx = (ix - x) * (ix - x);
            wt = EXP(-(xx + yy) / WTFAC);
            sum[ix][jy] += (wt * array[y][x]);
            weight[ix][jy] += wt;
          }
        }
      }

      if (y >= order) {
        jy = y1 % mag;
        for (x = 0; x < nx; x++) {
          if (weight[x][jy] > 0.0)
            array[y][x] = sum[x][jy] / weight[x][jy];
          sum[x][jy] = weight[x][jy] = 0.0;
        }
      }
    }

    for (y = ny - order; y < ny; y++) {    /* Write out the last rows. */
      jy = y % mag;
      for (x = 0; x < nx; x++) {
        if (weight[x][jy] > 0.0)
          array[y][x] = sum[x][jy] / weight[x][jy];
      }
    }

    freematrix(sum, 0, 0);         /* Release the local array storage. */
    freematrix(weight, 0, 0);

    return(0);
}
