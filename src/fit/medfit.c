/*
	<medfit.c> -- Code to perform median line fitting.
	24aug92 jm  Original code modified from Numerical Recipes.
	16sep92 jm  Modified to permit assignment of fit to
		    individual user variables.
	26jul93 jm  Added Gaussian fit.
	09aug93 jm  Removed sscan calls in favor of wipisnumber().
	29aug94 jm  Removed conditional in gaussfit() that would cause
		    an infinite loop whenever the output was re-entered
		    as input.  Also fixed bug in wipfit() that forces
		    the amplitude to always be fit (it was chosing the
		    width and not the amplitude).
	27sep95 jm  Permitted zero value to be fit in gaussfit().
        19jun96 jm  Split the fit code off into it's own individual
		    directory and split the different types of fits
		    into individual routines.

  NOTE:  This code was originally extracted from the Fortran version of
  Numerical Recipes by Press, et. al. and has been modified and updated
  to be used within WIP.  There are substantial enough differences in
  this code from the code that now appears in the newer versions of the
  Press book that the user should make no assumptions about the
  transportable nature of this code.  The bulk of the code in this file
  belongs to the authors of Numerical Recipes and should not be extracted.
  In other words, get your own book!

Routines:
static void hpsort ARGS(( unsigned long int n, float ra[] ));
static float rofunc ARGS(( float b, float x[], float y[], float arr[], \
  int npts, float *aa, float *abdev ));
int medfit ARGS(( float x[], float y[], int ndata, float *a, float *b, \
  float *abdev ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Sorts the array ra[] (of length n) into ascending numberical order
 *  using the Heapsort algorithm.  n" is input; ra is replaced on
 *  output by its sorted rearrangment.
 *  From NUMERICAL RECIPES (pg. 247), 8/24/92 jm.
 *  From NUMERICAL RECIPES in C 2nd Ed. (pg. 337), 7/23/93 jm.
 */
#ifdef PROTOTYPE
static void hpsort(unsigned long int n, float ra[])
#else
static void hpsort(n, ra)
unsigned long int n;
float ra[];
#endif /* PROTOTYPE */
{
    unsigned long int L, j, ir, i;
    float rra;

    if (n < 2)
      return;

    L = (n >> 1) + 1;
    ir = n;
    /*
     *  The index L will be decremented from its initial value down to
     *  0 during the "hiring" (heap creation) phase.  Once it reaches 0,
     *  the index ir will be decremented from its initial value down to
     *  0 during the "retirement-and-promotion" (heap selection) phase.
     */
    for (;;) {
      if (L > 1) {                           /* Still in hiring phase. */
        L--;
        rra = ra[L - 1];
      } else {                   /* In retirement-and-promotion phase. */
        ir--;
        rra = ra[ir];        /* Clear a space at the end of the array. */
        ra[ir] = ra[0];         /* Retire the top of the heap into it. */
        if (ir == 1) {                /* Done with the last promotion. */
          ra[0] = rra;                 /* The least competent element. */
          break;
        }
      }
      /*
       * Whether in hiring or promotion phase, this section
       *  sifts down element rra to its proper level.
       */
      i = L;
      j = L << 1;
      while (j <= ir) {
                            /* First, compare to the better underling. */
        if ((j < ir) && (ra[j - 1] < ra[j])) j++;
        if (rra < ra[j - 1]) {                          /* Demote rra. */
          ra[i - 1] = ra[j - 1];
          i = j;
          j <<= 1;
        } else {                               /* This is rra's level. */
          j = ir + 1;             /* Set j to terminate the sift-down. */
        }
      }
      ra[i - 1] = rra;                       /* Put rra into its slot. */
    }

    return;
}

#define EPS 1.0E-7

/*
 *  Returns the sum of the function (for a given input of b):
 *    x[i] * sgn(y[i] - a - b*x[i])    (i = 0, npts-1)
 *  where sgn(0) is 0.
 *
 *  Also returned is a new median value (aa) and a new estimate of
 *  the mean absolute deviation (abdev).
 *
 *  x[] and y[] are the data (both of size npts) and are only read;
 *  arr[] is a work array (also of size npts) and is modified.
 */
#ifdef PROTOTYPE
static float rofunc(float b, float x[], float y[], float arr[],
  unsigned long int npts, float *aa, float *abdev)
#else
static float rofunc(b, x, y, arr, npts, aa, abdev)
unsigned long int npts;
float b;
float x[], y[], arr[];
float *aa, *abdev;
#endif /* PROTOTYPE */
{
    unsigned long int j;
    float d, sum;

    for (j = 0; j < npts; j++)                 /* Fill the work array. */
      arr[j] = y[j] - (b * x[j]);

    hpsort(npts, arr);                         /* Sort the work array. */
    if (npts & 1) {                         /* Odd number of elements. */
      j = (npts - 1) >> 1;
      *aa = arr[j];
    } else {                               /* Even number of elements. */
      j = npts >> 1;
      *aa = 0.5 * (arr[j - 1] + arr[j]);
    }

    *abdev = sum = 0.0;
    for (j = 0; j < npts; j++) {
      d = y[j] - ((*aa) + (b * x[j]));
      *abdev += ABS(d);
      if (y[j] != 0.0) d /= ABS(y[j]);
      if (ABS(d) > EPS) sum += ((d < 0.0) ? -x[j] : x[j]);
    }

    return(sum);
}

#undef EPS

/*
 *  Fits "y = a + bx" by the criterion of least absolute deviations.
 *  The arrays x[] and y[] (of length ndata) are the input points to
 *  fit (and are not changed by this routine).  The fitted parameters
 *  (a and b) are output, along with abdev, which is the mean absolute
 *  deviation (in the y direction) of the points from the fitted line.
 *
 *  Returns 0 if successful; 1 if there are an insufficient number of
 *  points or it can't allocate the needed work array.
 */
#ifdef PROTOTYPE
int medfit(float x[], float y[], int ndata, float *a, float *b, float *abdev)
#else
int medfit(x, y, ndata, a, b, abdev)
float x[], y[], *a, *b, *abdev;
int ndata;
#endif /* PROTOTYPE */
{
    unsigned long int j, npts;
    float aa, bb, b1, b2, del, f, f1, f2, sigb, temp;
    float sx, sy, sxy, sxx, chisq;
    float *arr;

    if (ndata < 2) {
      wipoutput(stderr, "Not enough points to find a median fit.\n");
      return(1);
    }

    npts = ndata;

    /*  Allocate a local work array. */

    arr = (float *)Malloc((unsigned)(npts * sizeof(float)));
    if (arr == (float *)NULL) {
      wipoutput(stderr,
        "Trouble allocating a work array in the function medfit().\n");
      return(1);
    }

    sx = sy = sxy = sxx = chisq = 0.0;
    for (j = 0; j < npts; j++) {                  /* As a first guess, */
      sx += x[j];                                 /* "a" and "b" will  */
      sy += y[j];                                 /* be the result of  */
      sxy += (x[j] * y[j]);                       /* a least-squares   */
      sxx += (x[j] * x[j]);                       /* fit to the line.  */
    }

    del = (npts * sxx) - (sx * sx);
    aa = ((sxx * sy) - (sx * sxy)) / del;             /* Least-squares */
    bb = ((npts * sxy) - (sx * sy)) / del;               /* solutions. */

    for (j = 0; j < npts; j++) {    /* Compute the initial chi-square. */
      temp = y[j] - (aa + (bb * x[j]));
      chisq += (temp * temp);
    }

    /*
     * The standard deviation will give some idea of how big
     * an iteration step to take.
     */

    sigb = SQRT(chisq / del);
    b1 = bb;
    f1 = rofunc(b1, x, y, arr, npts, &aa, abdev);

    /*
     *  As a guess, bracket 3-sigma away in the downhill
     *  direction (known from the value of f1).
     */

    b2 = ABS(3.0 * sigb);
    b2 = bb + ((f1 < 0.0) ? -b2 : b2);
    f2 = rofunc(b2, x, y, arr, npts, &aa, abdev);

    /*
     *  Next if conditional added to take care of the case of a
     *  line input into this function.  It is needed to avoid an
     *  infinite loop when (b1 == b2) (within floating point error).
     */

    if (ABS(b2 - b1) > (sigb + 0.005)) {
      while ((f1 * f2) > 0.0) {                         /* Bracketing. */
        bb = (2.0 * b2) - b1;
        b1 = b2;
        f1 = f2;
        b2 = bb;
        f2 = rofunc(b2, x, y, arr, npts, &aa, abdev);
      }
    }

    /*
     * Refine until the error is a negligible number of
     * standard deviations.
     */

    sigb *= 0.01;
    while (ABS(b2 - b1) > sigb) {                        /* Bisection. */
      bb = 0.5 * (b1 + b2);
      if ((bb == b1) || (bb == b2)) break;
      f = rofunc(bb, x, y, arr, npts, &aa, abdev);
      if ((f * f1) >= 0.0) {
        f1 = f;
        b1 = bb;
      } else {
        f2 = f;
        b2 = bb;
      }
    }

    Free(arr);                              /* Free up the work array. */

    *a = aa;
    *b = bb;
    *abdev /= npts;

    return(0);
}
