/*
	<fit.c> -- Code to parse and perform various line fitting.
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
		    into individual routines.  Modified wipfit to use
		    the current world limits to delimit the range of
		    the fit applied.  Also added wipplotfit routine
		    to display the most recent fit.

Routines:
static double getfit ARGS(( float xval ));

void wipplotfit ARGS(( float x1, float x2, float step, float x[], int nx ));
void wipfitrange ARGS(( float x1, float x2, float y1, float y2 ));
int wipfit ARGS(( Const char *rest, int nxy, float x[], float y[], int ns, float sig[] ));
*/

#include "wip.h"

/* Global variables for just this file */

static enum {NoFit, Linear, Polynomial, Legendre, Gaussian} PreviousFit;
static int NumberOfGaussians = 0;
static float *FitTerms = (float *)NULL;
static int NumberOfPolynomials = 0;
static float *PolyFit = (float *)NULL;
static float LineFit[2];
static float XRange[2];
static float YRange[2];
static Logical XRangeLimited = FALSE;
static Logical YRangeLimited = FALSE;

/* Code */

#ifdef PROTOTYPE
static float getLegendre(float xval, float poly[], int npoly)
#else
static float getLegendre(xval, poly, npoly)
float xval;
float poly[];
int npoly;
#endif /* PROTOTYPE */
{
    int j;
    float twox, f1, f2, d;
    float pl[3];
    float yval;

    pl[1] = 1.0;
    pl[2] = xval;

    yval = pl[1] * poly[0];
    if (npoly == 1)
      return(yval);

    yval += (pl[2] * poly[1]);
    if (npoly == 2)
      return(yval);

    twox = 2.0 * xval;
    f2 = xval;
    d = 1.0;
    for (j = 2; j < npoly; j++) {
      f1 = d++;
      f2 += twox;
      pl[0] = pl[1];
      pl[1] = pl[2];
      pl[2] = ((f2 * pl[1]) - (f1 * pl[0])) / d;
      yval += (pl[2] * poly[j]);
    }

    return(yval);
}

#ifdef PROTOTYPE
static double getfit(float xval)
#else
static double getfit(xval)
float xval;
#endif /* PROTOTYPE */
{
    int j;
    double yval, expon;

    yval = 0.0;
    switch (PreviousFit) {
      case Linear:
        yval = LineFit[0] + (LineFit[1] * xval);
        break;
      case Polynomial:
        yval = PolyFit[0];
        expon = 1.0;
        for (j = 1; j < NumberOfPolynomials; j++) {
          expon *= xval;
          yval += (expon * PolyFit[j]);
        }
        break;
      case Legendre:
        yval = getLegendre(xval, PolyFit, NumberOfPolynomials);
        break;
      case Gaussian:
        for (j = 0; j < NumberOfGaussians; j += 3) {
          expon = (xval - FitTerms[j+1]) / FitTerms[j+2];
          yval += (FitTerms[j] * EXP(-expon * expon));
        }
        break;
      default:                       /* Should never reach this point! */
        wipoutput(stderr, "Something very wrong in plotfit routine.");
        break;
    }

    return(yval);
}

/*
 *  This routine plots the curve using the parameters from the most
 *  recent fit.  The inputs x1/x2 provide the range over which the
 *  plot will run.  These should probably default to the display x1/x2.
 *  The step size indicates the number of points to be plotted (ie. the
 *  resolution).  If the step size is not provided, then the x[] array
 *  (readonly) is used to guess a reasonable value.
 *
 *  This routine returns with a warning if no previous fit exists or
 *  if the previous fit terminated with an error.
 */
#ifdef PROTOTYPE
void wipplotfit(float x1, float x2, float step, float x[], int nx)
#else
void wipplotfit(x1, x2, step, x, nx)
float x1;
float x2;
float step;
float x[];
int nx;
#endif /* PROTOTYPE */
{
    int j, n;
    float xval, yval;

    if (PreviousFit == NoFit) {
      wipoutput(stderr, "No fit previously performed.");
      return;
    }

    /*
     *  If the step size is not provided but an array of X values is
     *  present, then compute the step size by taking the mean value
     *  of the spacing between individual array values.  If the X array
     *  is not provided, then use the provided limits to get a guess.
     */

    if (step == 0.0) {
      if (nx > 1) {
        step = 0.0;
        for (j = 1; j < nx; j++) {
          step += (x[j] - x[j-1]);
        }
        step /= (nx - 1.0);
      } else {
        step = (x2 - x1) / 10.0;
      }
    }

    n = (x2 - x1) / step;
    xval = x1;
    yval = getfit(xval);

    cpgbbuf();
    wipmove(xval, yval);

    for (j = 0; j < n; j++) {
      xval += step;
      yval = getfit(xval);
      wipdraw(xval, yval);
    }

    cpgebuf();

    return;
}

#ifdef PROTOTYPE
void wipfitrange(float x1, float x2, float y1, float y2)
#else
void wipfitrange(x1, x2, y1, y2)
float x1;
float x2;
float y1;
float y2;
#endif /* PROTOTYPE */
{
    if (x1 == x2) {
      XRangeLimited = FALSE;
    } else {
      XRange[0] = x1;
      XRange[1] = x2;
      XRangeLimited = TRUE;
    }

    if (y1 == y2) {
      YRangeLimited = FALSE;
    } else {
      YRange[0] = y1;
      YRange[1] = y2;
      YRangeLimited = TRUE;
    }

    return;
}

/*
 *  WIP Fitting master routine.  There are currently 5 different
 *  types of fitting:
 *    (a) Fits the line "y = a + bx" by the Least squares method ("lsqfit");
 *    (b) Fits the line "y = a + bx" by the Criterion of least
 *        absolute deviations ("medfit");
 *    (c) Fits a Polynomial of degree N (where N is an input parameter);
 *    (d) Fits a Legendre Polynomial of degree N (where N is an input);
 *    (e) Fits n-Gaussian curve(s) to the data ("gaussian").
 *
 *  The arrays x[] and y[] (of length nxy), are the input data to fit.
 *  The number of points in the sig[] array is ns (sig[] is not used by
 *  the medfit routine).  A warning is generated by the Polynomial and
 *  Gaussian fitting routines if the number of sig[] points (ns) does
 *  not equal the number of data points (nxy) but the fit continues
 *  using which ever valid error points are present.  The Least Squares
 *  Method also generates a warning if ns does not equal nxy but, in
 *  this case, the error values are ignored.
 *
 *  The input string (rest) specifies which routine to use and must be
 *  one of:
 *    > "lsqfit"
 *    > "medfit"
 *    > "polynomial"
 *    > "legendre"
 *    > "gaussian"
 *  (case-independent) and identifies the names of the user variables to
 *  associate with the fit parameters.  If fewer names are present than
 *  fit parameters returned, the remaining fit items are ignored (i.e.
 *  not assigned).  The fit and errors are always displayed on the
 *  standard output.
 *
 *  The routine "lsqfit" returns (in this order) the parameters a and b
 *  along with a chi-square for the fit and sigma errors on the
 *  constants a and b.  If the sig[] array was used to specify errors
 *  on the observation points, a "goodness of fit" is also returned.
 *  If no errors were specified, the correlation coefficient is
 *  returned in place of the "goodness of fit".
 *
 *  The routine "medfit" returns (in this order) the fit parameters
 *  a and b along with absdev (the mean absolute deviation in the
 *  y-direction).
 *
 *  The routines "polynomial" and "legendre" both require one additional
 *  argument: the number of polynomials to fit (i.e. one plus the order
 *  of the polynomial to fit).  The polynomial fitting routines return
 *  (in this order) each of the fit parameters of the polynomial
 *  followed by the error associated with each coefficient and then the
 *  chi-square of the fit.
 *
 *  For example, for a third order Legendre fit, one might use:
 *    WIP> fit legendre 4 \1 \2 \3 \4 \11 \12 \13 \14 \0
 *  This stores the polynomial coefficients in \1-\4, the error terms
 *  in \11-\14, and the chi-square value in \0.
 *
 *  The routine "gaussfit" requires one additional argument: the number
 *  of Gaussian curves to fit.  For each Gaussian, there are three terms
 *  fit: the amplitude, the mean, and the FWHM.  If the values input are
 *  negative, then the absolute value is used in the fit and this term
 *  is held fixed (i.e. not fit).  This function returns (in this order)
 *  the parameters amplitude, mean, and FWHM for each gaussian fit.  If
 *  the input is not a user variable or is not present, then it is still
 *  displayed on the standard output.  A final term may be present which
 *  is the returned chi-square of the fit.
 *
 *  For example:
 *    WIP> set \1 1.0   # Guess at amplitude.
 *    WIP> set \2 5.0   # First guess at X-position of peak.
 *    WIP> set \3 -2.3  # Input FHWM and hold this fixed during fit.
 *    WIP> fit gauss 1 \1 \2 \3 \4  # Fit 1 Gaussian and store fit and
 *                      # chi-square in \1 \2 \3 \4, respectively.
 *
 *  This routine returns 0 if successful; 1 on error.
 */
#ifdef PROTOTYPE
int wipfit(Const char *rest, int nxy, float x[], float y[], int ns, float sig[])
#else
int wipfit(rest, nxy, x, y, ns, sig)
Const char *rest;
int nxy, ns;
float x[], y[], sig[];
#endif /* PROTOTYPE */
{
    char *ptr, *type, *pvar;
    char string[STRINGSIZE];
    char uservar[STRINGSIZE];
    int j, npts, nsig;
    int nsize, nitem;
    size_t ntype;
    float *xptr, *yptr, *sptr;
    float *xvec, *yvec, *svec;
    float a, b, siga, sigb, chi2, q, absdev;
    double arg;
    Logical okay;

    PreviousFit = NoFit;

    if (wipDebugMode() > 0) {
      wipoutput(stdout, "Range clipping in the X direction is %s.\n",
        ((XRangeLimited == TRUE) ? "on" : "off"));
      if (XRangeLimited == TRUE)
        wipoutput(stdout, "Range limits in the X direction are [%G - %G].\n",
          XRange[0], XRange[1]);
      wipoutput(stdout, "Range clipping in the Y direction is %s.\n",
        ((YRangeLimited == TRUE) ? "on" : "off"));
      if (YRangeLimited == TRUE)
        wipoutput(stdout, "Range limits in the Y direction are [%G - %G].\n",
          YRange[0], YRange[1]);
    }

    xvec = yvec = svec = (float *)NULL;
    if ((XRangeLimited == TRUE) || (YRangeLimited == TRUE)) {
      xvec = vector(nxy);
      yvec = vector(nxy);
      if ((xvec == (float *)NULL) || (yvec == (float *)NULL)) {
        wipoutput(stderr, "Trouble allocating temporary array storage.\n");
        goto MISTAKE;
      }
      if ((ns > 0) && ((svec = vector(ns)) == (float *)NULL)) {
        wipoutput(stderr, "Trouble allocating temporary array storage.\n");
        goto MISTAKE;
      }
      xptr = xvec;
      yptr = yvec;
      sptr = svec;
      for (j = npts = nsig = 0; j < nxy; j++) {
        okay = FALSE;
        if ((XRangeLimited == TRUE) && (YRangeLimited == TRUE)) {
          a = (x[j] - XRange[0]) * (x[j] - XRange[1]);
          b = (y[j] - YRange[0]) * (y[j] - YRange[1]);
          if ((a <= 0) && (b <= 0)) okay = TRUE;
        } else if (XRangeLimited == TRUE) {
          a = (x[j] - XRange[0]) * (x[j] - XRange[1]);
          if (a <= 0) okay = TRUE;
        } else {   /* (YRangeLimited == TRUE) */
          a = (y[j] - YRange[0]) * (y[j] - YRange[1]);
          if (a <= 0) okay = TRUE;
        }
        if (okay == TRUE) {
          xvec[npts] = x[j];
          yvec[npts] = y[j];
          npts++;
          if (j < ns) svec[nsig++] = sig[j];
        }
      }
    } else {
      xptr = &x[0];
      yptr = &y[0];
      npts = nxy;
      sptr = &sig[0];
      nsig = ns;
    }

    if (npts < 2) {
      wipoutput(stderr, "Not enough points to fit.\n");
      goto MISTAKE;
    }

    if ((XRangeLimited == TRUE) || (YRangeLimited == TRUE)) {
      if (npts != nxy) {
        wipoutput(stdout,
          "Clipping decreased the number of points to fit from [%d] to [%d].\n",
          nxy, npts);
      } else {
        wipoutput(stdout,
          "Clipping did not change the number of points to fit.\n");
      }
    }

    ptr = Strncpy(string, rest, STRINGSIZE);     /* Make a local copy. */
    string[STRINGSIZE-1] = Null;        /* Make sure it is terminated. */
    if ((type = wipparse(&ptr)) == (char *)NULL)      /* Nothing here! */
      goto MISTAKE;
    wiplower(type);                             /* Make it lower case! */
    ntype = wiplenc(type);                   /* Get the string length. */

    if (Strncmp("lsqfit", type, ntype) == 0) {
      if (lsqfit(xptr, yptr, npts, sptr, nsig, &a, &b, &siga, &sigb, &chi2, &q)) {
        wipoutput(stderr, "Trouble finding a least-squares fit.\n");
        goto MISTAKE;
      }

      PreviousFit = Linear;
      LineFit[0] = a;
      LineFit[1] = b;

      wipoutput(stdout, "Fit parameters for y = a + (b * x):\n");
      wipoutput(stdout, "a = %G ", a);
      wipoutput(stdout, "b = %G ", b);
      wipoutput(stdout, "chi2 = %G ", chi2);
      wipoutput(stdout, "\n");
      wipoutput(stdout, "siga = %G ", siga);
      wipoutput(stdout, "sigb = %G ", sigb);
      if (nsig > 0) wipoutput(stdout, "q = %G ", q);
      else          wipoutput(stdout, "r = %G ", q);
      wipoutput(stdout, "\n");

      for (nitem = 0; ((pvar = wipparse(&ptr)) != (char *)NULL); nitem++) {
        switch (nitem) {
          case 0:
            SPrintf(uservar, "%s %G", pvar, a);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 1:
            SPrintf(uservar, "%s %G", pvar, b);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 2:
            SPrintf(uservar, "%s %G", pvar, chi2);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 3:
            SPrintf(uservar, "%s %G", pvar, siga);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 4:
            SPrintf(uservar, "%s %G", pvar, sigb);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 5:
            SPrintf(uservar, "%s %G", pvar, q);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          default:
            wipoutput(stderr, "Extra arguments ignored.\n");
            ptr = (char *)NULL;
            break;
        }
      }
    } else if (Strncmp("medfit", type, ntype) == 0) {
      if (medfit(xptr, yptr, npts, &a, &b, &absdev)) {
        wipoutput(stderr, "Trouble finding a least absolute deviations fit.\n");
        goto MISTAKE;
      }

      PreviousFit = Linear;
      LineFit[0] = a;
      LineFit[1] = b;

      wipoutput(stdout, "Fit parameters for y = a + (b * x):\n");
      wipoutput(stdout, "a = %G ", a);
      wipoutput(stdout, "b = %G ", b);
      wipoutput(stdout, "absdev = %G ", absdev);
      wipoutput(stdout, "\n");

      for (nitem = 0; ((pvar = wipparse(&ptr)) != (char *)NULL); nitem++) {
        switch (nitem) {
          case 0:
            SPrintf(uservar, "%s %G", pvar, a);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 1:
            SPrintf(uservar, "%s %G", pvar, b);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 2:
            SPrintf(uservar, "%s %G", pvar, absdev);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          default:
            wipoutput(stderr, "Extra arguments ignored.\n");
            ptr = (char *)NULL;
            break;
        }
      }
    } else if ((Strncmp("polynomial", type, ntype) == 0) ||
               (Strncmp("legendre", type, ntype) == 0)) {
      if (wiparguments(&ptr, 1, &arg) != 1) {
        wipoutput(stderr,
          "Number of Polynomial coefficients to fit must be input.\n");
        goto MISTAKE;
      }

      if ((nsig > 0) && (nsig != npts)) {
        wipoutput(stderr, "Warning: %s\n",
          "number of data points doesn't equal the number of error points.");
      }

      nsize = NINT(arg);
      nitem = (Strncmp("legendre", type, ntype) == 0);

      /*
       *  If the previous fit exists but was a different size, then
       *  free up the previous storage prior to allocating the new size.
       */

      if (nsize != NumberOfPolynomials) {
        if (PolyFit != (float *)NULL)
          Free(PolyFit);
        PolyFit = (float *)Malloc((unsigned)(nsize) * sizeof(float));
        if (PolyFit == (float *)NULL) {
          wipoutput(stderr,
            "Trouble finding storage for the Polynomial fit arguments.\n");
          NumberOfPolynomials = 0;
          goto MISTAKE;
        }
        NumberOfPolynomials = nsize;
      }

      if (wippolyfit(ptr, xptr, yptr, npts, sptr, nsig, nitem, nsize, PolyFit))
        goto MISTAKE;

      if (nitem == 0)
        PreviousFit = Polynomial;
      else
        PreviousFit = Legendre;
    } else if (Strncmp("gaussian", type, ntype) == 0) {
      if (wiparguments(&ptr, 1, &arg) != 1) {
        wipoutput(stderr, "Number of Gaussian curves to fit must be input.\n");
        goto MISTAKE;
      }

      if ((nsig > 0) && (nsig != npts)) {
        wipoutput(stderr, "Warning: %s\n",
          "number of data points doesn't equal the number of error points.");
      }

      nsize = 3 * NINT(arg);
      if (nsize <= 0) nsize = 3;

      /*
       *  If the previous fit exists but was a different size, then
       *  free up the previous storage prior to allocating the new size.
       */

      if (nsize != NumberOfGaussians) {
        if (FitTerms != (float *)NULL)
          Free(FitTerms);
        FitTerms = (float *)Malloc((unsigned)(nsize) * sizeof(float));
        if (FitTerms == (float *)NULL) {
          wipoutput(stderr,
            "Trouble finding storage for the Gaussian fit arguments.\n");
          NumberOfGaussians = 0;
          goto MISTAKE;
        }
        NumberOfGaussians = nsize;
      }

      if (wipgaussfit(ptr, xptr, yptr, npts, sptr, nsig, (nsize / 3), FitTerms))
        goto MISTAKE;

      PreviousFit = Gaussian;
    } else {
      wipoutput(stderr, "Unknown fit type: [%s].\n", type);
      goto MISTAKE;
    }

    return(0);

MISTAKE:
    if (xvec != (float *)NULL) freevector(xvec);
    if (yvec != (float *)NULL) freevector(yvec);
    if (svec != (float *)NULL) freevector(svec);
    return(1);
}
