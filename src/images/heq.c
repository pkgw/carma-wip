/*
 *  wipheq -- Histogram equalize an image.
 *
 *  History:
 *     27nov95  jm Neil's original Fortran code converted to C.
 *     28oct96  jm Modified heavily so that the image is now readonly;
 *                 now only the color palette is changed.
 *
 *  Routines:
 *    void wipheq ARGS(( int nx, int ny, float **image,
 *                      int x1, int x2, int y1, int y2,
 *                      float blank, float min, float max, int nbins ));
 */

#include "wip.h"

/* Global Variables needed just for this file */

#define SFAC 65000.0

/* Code */

/*
 *  Apply histogram equalization to an image directly.
 *  Uses the color index minimum and maximum as well as the
 *  image transfer function (linear, sqrt, etc.).
 *
 *  Input:
 *   nx,ny  Number of pixels along each axis.
 *   image  Image data.
 *   x1,x2  Pixel subrange in the X direction.
 *   y1,y2  Pixel subrange in the Y direction.
 *   min    Display minimum intensity range.
 *   max    Display maximum intensity range.
 *   blank  Values less than this threshold are omitted from the histogram.
 *   nbins  Number of bins to partition the data into the histogram.
 */
#ifdef PROTOTYPE
void wipheq(int nx, int ny, float **image, int x1, int x2, int y1, int y2,
  float blank, float min, float max, int nbins)
#else
void wipheq(nx, ny, image, x1, x2, y1, y2, blank, min, max, nbins)
int nx;
int ny;
float **image;
int x1;
int x2;
int y1;
int y2;
float blank;
float min;
float max;
int nbins;
#endif /* PROTOTYPE */
{
    int idx, i, j;
    int itf, npix;
    int minind, maxind, ncolors;
    float fac, cum;
    float bv, bmin, bmax;
    int *his;
    float *cumhis, *r, *g, *b;
    double arg, sfacl;
    LOGICAL error;

    /* Check inputs. */

    if (nbins < 2) {
      wipoutput(stderr, "Number of histogram bins [%d] is too small.\n", nbins);
      return;
    }

    if ((nx <= 0) || (ny <= 0)) {
      wipoutput(stderr, "Image size is incorrect: [%d x %d].\n", nx, ny);
      return;
    }

    if ((x2 <= x1) || (y2 <= y1)) {
      wipoutput(stderr,
        "Subimage size is incorrect: [%d-%d] x [%d-%d].\n", x1, x2, y1, y2);
      return;
    }

    if ((x1 < 1) || (x2 > nx)) {
      wipoutput(stderr,
        "X-Subimage range is incorrect: 1<=[%d-%d]<=%d.\n", x1, x2, nx);
      return;
    }

    if ((y1 < 1) || (y2 > ny)) {
      wipoutput(stderr,
        "Y-Subimage range is incorrect: 1<=[%d-%d]<=%d.\n", y1, y2, ny);
      return;
    }

    if (min == max) {
      wipoutput(stderr, "Image minimum and maximum can not be equal.\n");
      return;
    }

    /* Get the current color levels and rgb values. */

    wipgetcir(&minind, &maxind);
    ncolors = maxind - minind + 1;

    if (ncolors < 2) {
      wipoutput(stderr,
        "The number of color levels [%d] available is too small.\n", ncolors);
      return;
    }

    r = vector(ncolors);
    g = vector(ncolors);
    b = vector(ncolors);

    if ((r == (float *)NULL) || (g == (float *)NULL) || (b == (float *)NULL)) {
      if (r != (float *)NULL) freevector(r);
      if (g != (float *)NULL) freevector(g);
      if (b != (float *)NULL) freevector(b);
      wipoutput(stderr,
        "Trouble allocating color element arrays in wipheq().\n");
      return;
    }

    for (j = 0, i = minind; i <= maxind; j++, i++) {
      cpgqcr(i, &r[j], &g[j], &b[j]);
    }

    /* Get and initialize the histogram arrays. */

    cumhis = vector(nbins);
    his = (int *)Malloc((unsigned)(nbins) * sizeof(int));
    if ((his == (int *)NULL) || (cumhis == (float *)NULL)) {
      if (his != (int *)NULL) Free(his);
      if (cumhis != (float *)NULL) freevector(cumhis);
      wipoutput(stderr, "Trouble allocating work arrays in wipheq().\n");
      return;
    }

    wipoutput(stdout,
      "Modifying the color palette using histogram equalization (%d bins).\n",
      nbins);

    bmin = min;
    bmax = max;
    for (i = 0; i < nbins; i++) {
      his[i] = 0;
      cumhis[i] = 0.0;
    }

    arg = wipgetvar("itf", &error);
    itf = (error == TRUE) ? 0 : NINT(arg);

    /* Generate image histogram. */

    fac = bmax - bmin;
    sfacl = LOG(1.0 + SFAC);
    npix = 0;

    for (j = y1 - 1; j < y2; j++) {   /* Sub-images are 1 index based. */
      for (i = x1 - 1; i < x2; i++) {
        if (image[j][i] > blank) {      /* Only good values used here. */
          bv = image[j][i];
          if (bmax > bmin) {
            if ((bv < bmin) || (bv > bmax)) continue;
            bv = (bv < bmin) ? bmin : (bv > bmax) ? bmax : bv ;
          } else {
            bv = (bv < bmax) ? bmax : (bv > bmin) ? bmin : bv ;
          }
          arg = (bv - bmin) / fac;
          switch (itf) {
            case 0: /* Linear */
              idx = ((float)(nbins - 1) * arg) + 0.5;
              break;
            case 1: /* Logarithmic */
              arg = 1.0 + (SFAC * ABS(arg));
              idx = ((float)(nbins - 1) * LOG(arg) / sfacl) + 0.5;
              break;
            case 2: /* Square Root */
              idx = ((float)(nbins - 1) * SQRT(ABS(arg))) + 0.5;
              break;
            default:
              idx = 0;
              break;
          }
          idx = (idx < 0) ? 0 : (idx > (nbins - 1)) ? nbins - 1 : idx;
          his[idx]++;
          npix++;
        }
      }
    }

    /* Generate cumulative histogram. */

    cum = 0.0;
    for (i = 0; i < nbins; i++) {
      cum += his[i];
      cumhis[i] = cum;
    }

    /* Now discretize the cumulative histogram values. */

    fac = (float)(maxind - minind) / (float)(npix);
    for (i = 0; i < nbins; i++) {
      /*
       * This index converts the actual cumulative histogram
       * value to the nearest discrete bin.
       */
      idx = minind + ((cumhis[i] * fac) + 0.5);
      idx = (idx < minind) ? minind : (idx > maxind) ? maxind : idx;

      /* Convert this bin back to an intensity and reuse CUMHIS array. */

      cumhis[i] = idx - minind;
    }

    fac = (float)(nbins - 1) / (float)(maxind - minind);
    for (j = minind; j <= maxind; j++) {
      idx = ((j - minind) * fac) + 0.5;
      idx = cumhis[idx];
      cpgscr(j, r[idx], g[idx], b[idx]);
      i = j - minind;
      if (wipDebugMode() > 0)
        wipoutput(stdout, "%3d -> %3d  %.2f %.2f %.2f -> %.2f %.2f %.2f\n",
          j, (idx + minind), r[i], g[i], b[i], r[idx], g[idx], b[idx]);
    }

#if 0
    if (wipDebugMode() > 0) {
      int id;
      float ymin, ymax, y1, y2;
      float *xvec;
      if ((xvec = vector(nbins)) != (float *)NULL) {
        for (i = 0; i < nbins; i++) xvec[i] = i;
        if (id = cpgopen("/xs")) {
          cpgpap(2.5, 2.0);
          cpgsch(1.2);
          cpgsubp(1, 2);
          cpgscr(0, 0.4, 0.4, 0.4);

          wiprange(nbins, cumhis, &ymin, &ymax);
          cpgrnge(ymin, ymax, &y1, &y2);
          cpgenv(0.0, (float)nbins, y1, y2, 0, 0);
          cpgbin(nbins, xvec, cumhis, 1);

          for (i = 0; i < nbins; i++) cumhis[i] = his[i];
          wiprange(nbins, cumhis, &ymin, &ymax);
          cpgrnge(ymin, ymax, &y1, &y2);
          cpgenv(0.0, (float)nbins, y1, y2, 0, 0);
          cpgbin(nbins, xvec, cumhis, 1);

          cpgclos();
          cpgslct(1); /* A guess... */
        }
        freevector(xvec);
      }
    }
#endif

    Free(his);
    freevector(cumhis);
    freevector(b);
    freevector(g);
    freevector(r);

    return;
}
