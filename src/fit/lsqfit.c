/*
	<lsqfit.c> -- Code to perform various line fitting.
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
static double gammln ARGS(( float x ));
static float gcf ARGS(( float a, float x ));
static float gser ARGS(( float a, float x ));
static float gammq ARGS(( float a, float x ));

int lsqfit ARGS(( float x[], float y[], int ndata, float sig[], int mwt,\
  float *a, float *b, float *siga, float *sigb, float *chi2, float *q ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  This returns the value ln(GAMMA(X)) for X > 0.
 *  From NUMERICAL RECIPES (pg. 168), 9/14/88 jm.
 *  From NUMERICAL RECIPES in C 2nd Ed. (pg. 214), 7/23/93 jm.
 */
#ifdef PROTOTYPE
static double gammln(float x)
#else
static double gammln(x)
float x;
#endif /* PROTOTYPE */
{
    register int j;
    double y, tmp, ser;
    static double cof[6] = {76.18009172947146,     -86.50532032941677,
                            24.01409824083091,      -1.231739572450155,
                             0.1208650973866179E-2, -0.5395239384953E-5};

    y = x;
    tmp = x + 5.5;
    tmp -= ((x + 0.5) * LOG(tmp));
    ser = 1.000000000190015;

    for (j = 0; j < 6; j++) {
      ser += (cof[j] / (++y));
    }

    ser *= (2.5066282746310005 / x);
    return(LOG(ser) - tmp);
}

#define ITMAX 100             /* Maximum allowed number of iterations. */
#define EPS   3.0E-7                             /* Relative accuracy. */
#define FPMIN 1.0E-30
      /* Number near the smallest representable floating-point number. */

/*
 *  Returns the incomplete gamma function Q(a,x) evaluated by its
 *  continued fraction representation.
 *  From NUMERICAL RECIPES (pg. 174), 9/14/88 jm.
 *  From NUMERICAL RECIPES in C 2nd Ed. (pg. 219), 7/23/93 jm.
 */
#ifdef PROTOTYPE
static float gcf(float a, float x)
#else
static float gcf(a, x)
float a, x;
#endif /* PROTOTYPE */
{
    register int n;
    float an, b, c, d, del, f;
    double gln;

    gln = gammln(a);
    b = x + 1.0 - a;
    c = 1.0 / FPMIN;
    f = d = 1.0 / b;

    for (n = 1; n <= ITMAX; n++) {
      an = -n * (n - a);
      b += 2.0;
      d = (an * d) + b;
      if (ABS(d) < FPMIN) d = FPMIN;
      c = (an / c) + b;
      if (ABS(c) < FPMIN) c = FPMIN;
      d = 1.0 / d;
      del = c * d;
      f *= del;
      if (ABS(del - 1.0) < EPS) break;
    }

    if (n > ITMAX) {
      wipoutput(stderr, "a is too large or ITMAX too small in routine GCF.\n");
      return(0.0);
    }

    f *= EXP(-x + (a * LOG(x)) - gln);
    return(f);
}

/*
 *  Returns the incomplete gamma function P(a,x) evaluated by its
 *  series representation.
 *  From NUMERICAL RECIPES (pg. 173), 9/14/88 jm.
 *  From NUMERICAL RECIPES in C 2nd Ed. (pg. 218), 7/23/93 jm.
 */
#ifdef PROTOTYPE
static float gser(float a, float x)
#else
static float gser(a, x)
float a, x;
#endif /* PROTOTYPE */
{
    register int n;
    float sum, del, ap;
    double gln;

    gln = gammln(a);
    if (x <= 0.0) {
      if (x < 0.0)
        wipoutput(stderr, "x less than 0 in routine GSER\n");
      return(0.0);
    } else {
      ap = a;
      del = sum = 1.0 / a;
      for (n = 0; n < ITMAX; n++) {
        ap += 1.0;
        del *= (x / ap);
        sum += del;
        if (ABS(del) < (ABS(sum) * EPS)) {
          sum *= EXP(-x + (a * LOG(x)) - gln);
          return(sum);
        }
      }
      wipoutput(stderr, "a is too large or ITMAX too small in routine GSER.\n");
      return(0.0);
    }
}

#undef ITMAX
#undef EPS
#undef FPMIN

/*
 *  This returns the incomplete gamma function Q(a,x) = 1 - P(a,x).
 *  GSER returns the series representation of P(a,x).
 *  GCF returns the continued fraction representation of Q(a,x).
 *  From NUMERICAL RECIPES (pg. 173), 9/14/88 jm.
 *  From NUMERICAL RECIPES in C 2nd Ed. (pg. 218), 7/23/93 jm.
 */
#ifdef PROTOTYPE
static float gammq(float a, float x)
#else
static float gammq(a, x)
float a, x;
#endif /* PROTOTYPE */
{
    float gamser, gammcf;

    if ((x < 0.0) || (a <= 0.0)) {
      wipoutput(stderr, "Invalid arguments in routine GAMMQ.\n");
      return(0.0);
    }

    if (x < (a + 1.0)) {             /* Use the series representation. */
      gamser = gser(a, x);
      return(1.0 - gamser);
    } else {             /* Use the continued fraction representation. */
      gammcf = gcf(a, x);
      return(gammcf);
    }
}

/*
 *  Given a set of NDATA points X[I], Y[I] with standard deviations
 *  SIG[I], fit them to the straight line y = (a + bx) by minimizing
 *  CHI2.  Returned are A, B and their respective probable
 *  uncertainties SIGA and SIGB, the chi-square of the fit CHI2,
 *  and the goodness-of-fit probability Q (probability that the
 *  fit would have CHI2 this large or larger).
 *
 *  MWT is the number of points in the sig[] array.  MWT should either
 *  be equal to NDATA or set to 0.  If MWT = 0, then the standard
 *  deviations are assumed to be unavailable.  For this case, Q is
 *  returned with as the value of r, the correlation coefficient, and
 *  the normalization of CHI2 is to unit standard deviation on all points.
 *
 *  Returns 0 to conform with call syntax; no other status available.
 *
 *  14sep88 jm  Originally extracted from the Fortran version of
 *              NUMERICAL RECIPES code fit (pg. 508) (C: pg. 665).
 *  22aug92 jm  Modified the C code to 0 based arrays and removed
 *              redundant parameters.  Changed meaning of MWT to
 *              indicate the number of elements in the sig[] array.
 *  23jul93 jm  Modified to return r in q slot for mwt=0.
 */
#ifdef PROTOTYPE
int lsqfit(float x[], float y[], int ndata, float sig[], int mwt, float *a,
  float *b, float *siga, float *sigb, float *chi2, float *q)
#else
int lsqfit(x, y, ndata, sig, mwt, a, b, siga, sigb, chi2, q)
float x[], y[], sig[];
int ndata, mwt;
float *a, *b, *siga, *sigb, *chi2, *q;
#endif /* PROTOTYPE */
{
    register int i;
    float t, ss, sx, sy, sxoss, st2, sigdat;

    if ((mwt > 0) && (mwt != ndata)) {
      wipoutput(stderr, 
        "The number of uncertainties does not equal the number of points.\n");
      wipoutput(stderr, "The uncertainties will be ignored.\n");
      mwt = 0;
    } else if (mwt < 0) {
      wipoutput(stderr, 
        "The number of uncertainties can not be a negative number.\n");
      wipoutput(stderr, "All uncertainties will be ignored.\n");
      mwt = 0;
    }

    ss = sx = sy = 0.0;
    if (mwt > 0) {
      for (i = 0; i < ndata; i++) {
        t = 1.0 / (sig[i] * sig[i]);
        ss += t;
        sx += (x[i] * t);
        sy += (y[i] * t);
      }
    } else {
      for (i = 0; i < ndata; i++) {
        sx += x[i];
        sy += y[i];
      }
      ss = ndata;
    }

    sxoss = sx / ss;
    *b = st2 = 0.0;
    if (mwt > 0) {
      for (i = 0; i < ndata; i++) {
        t = (x[i] - sxoss) / sig[i];
        st2 += (t * t);
        *b += (t * y[i] / sig[i]);
      }
    } else {
      for (i = 0; i < ndata; i++) {
        t = x[i] - sxoss;
        st2 += (t * t);
        *b += (t * y[i]);
      }
    }

    /* Solve for a, b, siga, and sigb. */

    *b /= st2;
    *a = (sy - (sx * (*b))) / ss;
    *siga = SQRT((1.0 + ((sx * sx) / (ss * st2))) / ss);
    *sigb = 1.0 / SQRT(st2);

    /* Calculate chi-squared. */

    *chi2 = 0.0;
    if (mwt > 0) {
      for (i = 0; i < ndata; i++) {
        t = (y[i] - (*a) - ((*b) * x[i])) / sig[i];
        *chi2 += (t * t);
      }
      *q = gammq(0.5 * (ndata - 2), 0.5 * (*chi2));
    } else {
      ss = sy / (float)ndata;
      *q = 0.0;
      for (i = 0; i < ndata; i++) {
        t = y[i] - (*a) - ((*b) * x[i]);
        *chi2 += (t * t);
        *q += ((y[i] - ss) * (y[i] - ss));
      }
      *q = SQRT(1.0 - (*chi2 / *q));       /* In C book: (Eq. 15.2.13) */
      sigdat = SQRT((*chi2) / (float)(ndata - 2));
      *siga *= sigdat;
      *sigb *= sigdat;
    }

    return(0);
}
