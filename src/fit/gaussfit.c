/*
	<gaussfit.c> -- Code to perform gaussian fitting.
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
        30nov00 sally/pjt  fixed correct reporting of fwhm
         1dec00 pjt ansi-c; changed fixing parameters to the MAGICFIX character

  NOTE:  This code was originally extracted from the Fortran version of
  Numerical Recipes by Press, et. al. and has been modified and updated
  to be used within WIP.  There are substantial enough differences in
  this code from the code that now appears in the newer versions of the
  Press book that the user should make no assumptions about the
  transportable nature of this code.  The bulk of the code in this file
  belongs to the authors of Numerical Recipes and should not be extracted.
  In other words, get your own book!

Routines:
static void fgauss ARGS(( float x, float a[], float *y, float dyda[], int na ));
static void mrqcof ARGS(( float x[], float y[], float sig[], int ndata, \
  float a[], int ia[], int ma, float **alpha, float beta[], float *chisq, \
  void (*funcs) ARGS(( float, float [], float *, float [], int )) ));
static int gaussj ARGS(( float **a, int n, float **b, int m ));
static void covsrt ARGS(( float **covar, int ma, int ia[], int mfit ));
static int mrqmin ARGS(( float x[], float y[], float sig[], int ndata, \
  float a[], int ia[], int ma, float **covar, float **alpha, float *chisq, \
  void (*funcs) ARGS(( float, float [], float *, float [], int )), \
  float *alamda ));
static int gaussfit ARGS(( float x[], float y[], int ndata, \
  float sig[], int nsig, float a[], int ia[], int ma, float *chisq ));
static int moment ARGS(( float x[], float y[], int n, float *amp, \
  float *xmean, float *sdev ));

int wipgaussfit ARGS(( char *rest, float x[], float y[], int nxy, \
  float sig[], int nsig, int ngauss, float terms[] ));
*/

/*   undef this if you *must* be in the old incompatible mode */
#define MAGICFIX  '~'

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  y(x; a) is the sum of na/3 Gaussians.  The amplitude, center, and
 *  width of the Gaussians are stored in consecutive locations of a:
 *  a[i] = B[k], a[i+1] = E[k], a[i+2] = G[k] for k = 1, ..., na/3.
 *  Where:
 *    y(x) = B[k] * exp[-((x - E[k]) / G[k])**2]  (for k = 1, ..., na/3)
 *
 *  Both a[] and dyda[] are of size na and are 1-based vectors.
 */
static void fgauss(float x, float a[], float *y, float dyda[], int na)
{
    register int i;
    float fac, ex, arg;

    *y = 0.0;
    for (i = 1; i <= na - 1; i += 3) {
      arg = (x - a[i + 1]) / a[i + 2];
      ex = EXP(-arg * arg);
      fac = a[i] * ex * 2.0 * arg;
      *y += (a[i] * ex);
      dyda[i] = ex;
      dyda[i + 1] = fac / a[i + 2];
      dyda[i + 2] = fac * arg / a[i + 2];
    }
}

/*
 *  Used to evaluate the linearized fitting matrix alpha and vector beta
 *  and to calculate chi-squared.
 */

static void mrqcof(float x[], float y[], int ndata, float sig[], int nsig,
  float a[], int ia[], int ma, float **alpha, float beta[], float *chisq,
  void (*funcs)(float, float [], float *, float [], int))
{
    int i, j, k, L, m, mfit;
    float ymod, wt, sig2i, dy;
    float *dyda;

    mfit = 0;
    *chisq = 0.0;

    if ((dyda = vector(ma)) == (float *)NULL)
      return;
    dyda--;                 /* Convert this to a 1-index based vector. */

    for (j = 1; j <= ma; j++)
      if (ia[j]) mfit++;

                  /* Initialize the (symmetric) alpha and beta arrays. */
    for (j = 1; j <= mfit; j++) {
      for (k = 1; k <= j; k++)
        alpha[j][k] = 0.0;
      beta[j] = 0.0;
    }

    for (i = 1; i <= ndata; i++) {
      (*funcs)(x[i], a, &ymod, dyda, ma);
      sig2i = (i > nsig) ? 1.0 : ((ABS(sig[i]) < 0.0001) ? 0.0001 : sig[i]);
      sig2i = 1.0 / (sig2i * sig2i);
      dy = y[i] - ymod;
      for (j = 0, L = 1; L <= ma; L++) {
        if (ia[L]) {
          wt = dyda[L] * sig2i;
          for (j++, k = 0, m = 1; m <= L; m++)
            if (ia[m])
              alpha[j][++k] += (wt * dyda[m]);
          beta[j] += (dy * wt);
        }
      }
      *chisq += (dy * dy * sig2i);                /* Find chi-squared. */
    }

    for (j = 2; j <= mfit; j++) {       /* Fill in the symmetric side. */
      for (k = 1; k < j; k++)
        alpha[k][j] = alpha[j][k];
    }

    dyda++;                          /* Convert back to 0 based index. */
    freevector(dyda);
}

/*
 *  Linear equation solution by Gauss-Jordan elimination.
 *  a[1...n][1...n] is the input matrix.  b[1...n][1...m] is input
 *  containing the m right-hand side vectors.  On output, a is
 *  replaced by its matrix inverse and b is replaced by the
 *  corresponding set of solution vectors.
 *
 *  Returns 0 if successful; 1 on error.
 */
static int gaussj(float **a, int n, float **b, int m)
{
#define SWAP(a,b) {temp = (a); (a) = (b); (b) = temp;}

    int *indxc, *indxr, *ipiv;
    int status;
    int i, icol, irow, j, k, L, LL;
    float big, dum, pivinv, temp;

    status = 1;            /* To begin with, default this to an error. */

    indxc = indxr = ipiv = (int *)NULL;
    indxc = (int *)Malloc((unsigned)(n) * sizeof(int));
    indxr = (int *)Malloc((unsigned)(n) * sizeof(int));
    ipiv  = (int *)Malloc((unsigned)(n) * sizeof(int));
    if ((indxc == (int *)NULL) || (indxr == (int *)NULL) ||
        (ipiv  == (int *)NULL)) {
      if (ipiv  != (int *)NULL) Free(ipiv);
      if (indxr != (int *)NULL) Free(indxc);
      if (indxc != (int *)NULL) Free(indxr);
      wipoutput(stderr, "Trouble allocating storage for a work array.\n");
      return(status);
    }
    /* Shift these bookkeeping vectors to 1-based index. */
    indxc--;  indxr--;  ipiv--;

    for (j = 1; j <= n; j++)
      ipiv[j] = 0;

    for (i = 1; i <= n; i++) {     /* Loop over columns to be reduced. */
      big = 0.0;
      for (j = 1; j <= n; j++) {        /* Outer loop of pivot search. */
        if (ipiv[j] != 1) {
          for (k = 1; k <= n; k++) {
            if (ipiv[k] == 0) {
              dum = ABS(a[j][k]);
              if (dum >= big) {
                big = dum;
                irow = j;
                icol = k;
              }
            } else if (ipiv[k] > 1) {
              wipoutput(stderr, "gaussj(): Singular Matrix-1.\n");
              goto errBranch;
            }
          }
        }
      }

      ++(ipiv[icol]);
      /*
       *  The pivot element has been found, so interchange rows, if
       *  needed, to put the pivot element on the diagonal.  The
       *  columns are not physically interchanged, only relabeled:
       *  indxc[i], the column of the ith pivot element, is the ith
       *  column that is reduced, while indxr[i] is the row in which
       *  that pivot element was originally located.  If indxr[i] does
       *  not equal indxc[i] there is an implied column interchange.
       *  With this form of bookkeeping, the solution b's will end
       *  up in the correct order, and the inverse matrix will be
       *  scrambled by columns.
       */
      if (irow != icol) {
        for (L = 1; L <= n; L++) SWAP(a[irow][L], a[icol][L])
        for (L = 1; L <= m; L++) SWAP(b[irow][L], b[icol][L])
      }
      indxr[i] = irow;   /* Divide the pivot row by the pivot element. */
      indxc[i] = icol;   /* Pivot element is located at irow and icol. */
      if (a[icol][icol] == 0.0) {
        wipoutput(stderr, "gaussj(): Singular Matrix-2.\n");
        goto errBranch;
      }
      pivinv = 1.0 / a[icol][icol];
      a[icol][icol] = 1.0;
      for (L = 1; L <= n; L++) a[icol][L] *= pivinv;
      for (L = 1; L <= m; L++) b[icol][L] *= pivinv;
      for (LL = 1; LL <= n; LL++) {              /* Reduce the rows... */
        if (LL != icol) {              /* ...except for the pivot one. */
          dum = a[LL][icol];
          a[LL][icol] = 0.0;
          for (L= 1; L<= n; L++) a[LL][L] -= (a[icol][L] * dum);
          for (L= 1; L<= m; L++) b[LL][L] -= (b[icol][L] * dum);
        }
      }
    }

    /*
     *  This is the end of the main loop over columns of the reduction.
     *  It only remains to unscramble the solution in view of the column
     *  interchanges.  Do this by interchanging pairs of columns in the
     *  reverse order that the permutation was built up.
     */
    for (L = n; L >= 1; L--) {
      if (indxr[L] != indxc[L])
        for (k = 1; k <= n; k++)
          SWAP(a[k][indxr[L]], a[k][indxc[L]]);
    }

    status = 0;                              /* Set state to no error. */

errBranch:
    /* Shift these bookkeeping vectors back to 0-based index. */
    indxc++;  indxr++;  ipiv++;
    if (ipiv  != (int *)NULL) Free(ipiv);
    if (indxr != (int *)NULL) Free(indxc);
    if (indxc != (int *)NULL) Free(indxr);

    return(status);
#undef SWAP
}

/*
 *  Expand, in storage, the covariance matrix covar, so as to take
 *  into account parameters that are being help fixed.
 *  (For the latter, return zero covariances.)
 */
static void covsrt(float **covar, int ma, int ia[], int mfit)
{
#define SWAP(a,b) {swap = (a); (a) = (b); (b) = swap;}

    register int i, j, k;
    float swap;

    for (i = mfit + 1; i <= ma; i++)
      for (j = 1; j <= i; j++)
        covar[i][j] = covar[j][i] = 0.0;

    k = mfit;
    for (j = ma; j >= 1; j--) {
      if (ia[j]) {
        for (i = 1; i <= ma; i++) SWAP(covar[i][k], covar[i][j])
        for (i = 1; i <= ma; i++) SWAP(covar[k][i], covar[j][i])
        k--;
      }
    }

#undef SWAP
}

/*
 *  Levenberg-Marquardt method, attempting to reduce the value of
 *  chi-square of a fit between a set of data points x[1...ndata],
 *  y[1...ndata] with individual standard deviations sig[1...nsig],
 *  and a nonlinear function dependent on ma coefficients a[1...ma].
 *  The input array ia[1...ma] indicates by nonzero entries those
 *  components of a[] that should be fitted for, and by zero entries
 *  those conponents that should be held fixed at their input values.
 *
 *  The routine returns current best-fit values for the parameters
 *  a[1...ma] and chisq.  The arrays covar[1...ma][1...ma] and
 *  alpha[1...ma][1...ma] are used as working space during most
 *  iterations.
 *
 *  The supplied function funcs() evaluates the fitting function yfit
 *  and its derivatives dyda[1...ma] with respect to the fitting
 *  parameters a at x.
 *
 *  On the first call, provide an initial guess for the parameters a and
 *  set alamda < 0 for initialization (which then sets alamda = 0.001).
 *  If a step succeeds, chisq becomes smaller and alamda decreases by a
 *  factor of 10.  If a step fails, alamda grows by a factor of 10.
 *  This routine is called repeatedly until convergence is achieved.
 *  After that, this routine is a called one final time with alamda = 0,
 *  so that covar[1...ma][1...ma] returns the covariance matrix, and
 *  alpha[][], the curvature matrix.  (Parameters held fixed will return
 *  zero covariances.)
 *
 *  Returns 1 if there was trouble allocating arrays or some other
 *  error; 0 on success.
 */

static int mrqmin(float x[], float y[], int ndata, float sig[], int nsig,
  float a[], int ia[], int ma, float **covar, float **alpha, float *chisq,
  void (*funcs)(float, float [], float *, float [], int), float *alamda)
{
    int j, k, L, m;
    static int mfit;
    static float ochisq;
    static float *atry, *beta, *da;
    static float **oneda;

    if (*alamda < 0.0) {                             /* Initialization */
      for (mfit = 0, j = 1; j <= ma; j++)
        if (ia[j])
          mfit++;

      atry = vector(ma);
      if (atry == (float *)NULL) {
        wipoutput(stderr, "Trouble allocating work array storage.\n");
        return(1);
      }
      beta = vector(ma);
      if (beta == (float *)NULL) {
        wipoutput(stderr, "Trouble allocating work array storage.\n");
        freevector(atry);
        return(1);
      }
      da = vector(ma);
      if (da == (float *)NULL) {
        wipoutput(stderr, "Trouble allocating work array storage.\n");
        freevector(atry);
        freevector(beta);
        return(1);
      }
      oneda=matrix(1, mfit, 1, 1);
      if (oneda == (float **)NULL) {
        wipoutput(stderr, "Trouble allocating work array storage.\n");
        freevector(atry);
        freevector(beta);
        freevector(da);
        return(1);
      }
      atry--; beta--; da--;       /* Convert these to 1-based vectors. */

      *alamda = 0.001;
      mrqcof(x, y, ndata, sig, nsig, a, ia, ma, alpha, beta, chisq, funcs);
      ochisq = (*chisq);
      for (j = 1; j <= ma; j++)
        atry[j] = a[j];
    }

    /* Alter linearized fitting matrix, by augmenting diagonal elements. */
    for (j = 0, L = 1; L <= ma; L++) {
      if (ia[L]) {
        for (j++, k = 0, m = 1; m <= ma; m++) {
          if (ia[m]) {
            k++;
            covar[j][k] = alpha[j][k];
          }
        }
        covar[j][j] = alpha[j][j] * (1.0 + (*alamda));
        oneda[j][1] = beta[j];
      }
    }

    if (gaussj(covar, mfit, oneda, 1)) {           /* Matrix solution. */
      wipoutput(stderr, "Trouble in gaussj() routine.\n");
      freematrix(oneda, 1, 1);
      atry++; beta++; da++;  /* Convert these back to 0-based vectors. */
      freevector(da);
      freevector(beta);
      freevector(atry);
      return(1);
    }

    for (j = 1; j <= mfit; j++)
      da[j] = oneda[j][1];

    if (*alamda == 0.0) {                              /* Convergence. */
      covsrt(covar, ma, ia, mfit);      /* Evaluate covariance matrix. */
      freematrix(oneda, 1, 1);
      atry++; beta++; da++;  /* Convert these back to 0-based vectors. */
      freevector(da);
      freevector(beta);
      freevector(atry);
      return(0);
    }

    for (j = 0, L = 1; L <= ma; L++)         /* Did the trial succeed? */
      if (ia[L])
        atry[L] = a[L] + da[++j];

    mrqcof(x, y, ndata, sig, nsig, atry, ia, ma, covar, da, chisq, funcs);

    if (*chisq < ochisq) {       /* Success.  Accept the new solution. */
      *alamda *= 0.1;
      ochisq = (*chisq);
      for (j = 0, L = 1; L <= ma; L++) {
        if (ia[L]) {
          for (j++, k = 0, m = 1; m <= ma; m++) {
            if (ia[m]) {
              k++;
              alpha[j][k] = covar[j][k];
            }
          }
          beta[j] = da[j];
          a[L] = atry[L];
        }
      }
    } else {                  /* Failure.  Increase alamda and return. */
      *alamda *= 10.0;
      *chisq = ochisq;
    }

    return(0);
}

/*
 *  Returns 0 if successful; 1 otherwise.
 */
static int gaussfit(float x[], float y[], int ndata, float sig[], int nsig,
  float a[], int ia[], int ma, float *chisq)
{
    int j;
    float alamda, ochisq;
    float **covar, **alpha;

    if (ma < 1) {
      wipoutput(stderr, "No terms provided to Gaussian fit!\n");
      return(1);
    }

    covar = matrix(1, ma, 1, ma);
    alpha = matrix(1, ma, 1, ma);
    if ((covar == (float **)NULL) || (alpha == (float **)NULL)) {
      wipoutput(stderr, "Trouble allocating storage for the Gaussian fit.\n");
      goto errBranch;
    }

    alamda = -1;
    if (mrqmin(x-1, y-1, ndata, sig-1, nsig, a-1, ia-1, ma, covar, alpha,
      chisq, fgauss, &alamda)) {
      wipoutput(stderr, "Trouble making initial guess of the Gaussian fit.\n");
      goto errBranch;
    }

    for (j = 0; j < 10; /* NULL */) {        /* why 10  ????  - pjt */
      ochisq = *chisq;
      if (mrqmin(x-1, y-1, ndata, sig-1, nsig, a-1, ia-1, ma, covar, alpha,
        chisq, fgauss, &alamda)) {
        wipoutput(stderr, "Trouble solving for the Gaussian fit [%d].\n", j);
        goto errBranch;
      }
      if (*chisq > ochisq)
        j = 0;
      else if (ABS(ochisq - *chisq) < 0.1)
        j++;
    }

    alamda = 0.0;
    if (mrqmin(x-1, y-1, ndata, sig-1, nsig, a-1, ia-1, ma, covar, alpha,
      chisq, fgauss, &alamda)) {
      wipoutput(stderr, "Trouble solving for the final Gaussian fit.\n");
      goto errBranch;
    }

    wipoutput(stdout, "Gaussian fit uncertainties ([terms] are variances):\n");
    for (j = 1; j <= ma; j++) {
      if (covar[j][j] < 0.0) {
        wipoutput(stdout, "[%9.4f] ", covar[j][j]);
      } else {
        alamda = SQRT(covar[j][j]);
        wipoutput(stdout, "%9.4f ", alamda);
      }
    }

    freematrix(alpha, 1, 1);
    freematrix(covar, 1, 1);

    return 0;

errBranch:
    freematrix(alpha, 1, 1);
    freematrix(covar, 1, 1);

    return 1;
}

/*
 *  Finds the peak value of the y[] array, the mean value of the
 *  x[] array, and the standard deviation from the mean x[] array value.
 *  The latter two values are weighted by the y[] array values.
 *  The x[] and y[] arrays are not modified.
 *
 *  Returns 0 if successful; 1 if there are insufficient number of points.
 */
static int moment(float x[], float y[], int n, float *amp,
  float *xmean, float *sdev)
{
    register unsigned int j;
    float s, ep;
    float mom0, mom1, mom2;

    if (n < 2)
      return 1;

    *amp = y[0];
    ep = mom0 = mom1 = mom2 = 0.0;

    for (j = 0; j < n; j++) {
      if (y[j] > *amp) *amp = y[j];
      mom0 += y[j];
      mom1 += (y[j] * x[j]);
    }

    *xmean = mom1 / mom0;

    for (j = 0; j < n; j++) {
      s = x[j] - (*xmean);
      ep += (y[j] * s);
      mom2 += (y[j] * s * s);
    }

    mom2 = (mom2 - (ep * ep / mom0)) / (mom0 - 1);
    *sdev = (mom2 > 0) ? SQRT(mom2) : 0.0;

    return 0;
}

/*
 *  This performs the parsing and gaussian fitting.  The input string
 *  rest can contain suggestions for fit parameters and may also be
 *  used to store in user variables the found fit parameters.
 *  This string will be parsed by this routine!
 *  The x[] and y[] arrays contain nxy points to be fit.  The sig[]
 *  array contains nsig error bar values.  The term ngauss specifies
 *  the number of gaussians to fit.  The size of the work array terms[]
 *  is three times that size.  It is filled with the initial guesses
 *  and is returned with the fit parameters.
 *
 *  This routine returns 0 if successful; 1 otherwise.
 */
int wipgaussfit(char *rest, float x[], float y[], int nxy,
  float sig[], int nsig, int ngauss, float terms[])
{
    char *par, *pvar;
    char uservar[STRINGSIZE];
    int *ia;
    int j, nsize, idum;
    float a, b, q, chi2;
    double arg;

    nsize = 3 * ngauss;
    if (nsize <= 0) nsize = 3;

    ia = (int *)Malloc((unsigned)(nsize) * sizeof(int));
    if (ia == (int *)NULL) {
      wipoutput(stderr,
        "Trouble finding temporary storage for the Gaussian fit arguments.\n");
      return 1;
    }

    /* Process the input arguments (amp, mean, fwhm, ...). */
    par = Strncpy(uservar, rest, STRINGSIZE);     /* Make a local copy. */
    uservar[STRINGSIZE-1] = Null;       /* Make sure it is terminated. */
    for (j = 0; j < nsize; j++) {
      if ((pvar = wipparse(&par)) == (char *)NULL)
        break;
#ifdef MAGICFIX
      if (*pvar == MAGICFIX) {		/* keep this argument fixed for fitting */
	ia[j] = 0;
	pvar++;
      } else
	ia[j] = 1;
      if (wiparguments(&pvar, 1, &arg) != 1)
        break;
      terms[j] = arg;
#else
      if (wiparguments(&pvar, 1, &arg) != 1)
        break;
      ia[j] = (arg >= 0.0); 	/* Only fit if the argument is positive. */
      terms[j] = ABS(arg);
      if ((j % 3) == 0) {          /* The peak position is always fit. */
        ia[j] = 1;
        terms[j] = arg;
      }
#endif
    }

    if (j < nsize) {
      if (moment(x, y, nxy, &a, &b, &q)) {
        a = y[nxy / 2];
        b = x[nxy / 2];
        q = x[3 * nxy / 4] - x[nxy / 4];
        q = ABS(q);
      }

      while (j < nsize) {        /* Fill the rest with default values. */
        switch (j % 3) {
          case 0:                           /* Default peak amplitude. */
            terms[j] = a;
            break;
          case 1:                            /* Default peak position. */
            terms[j] = b;
            break;
          case 2:               /* Default full width at half maximum. */
            terms[j] = q;
            break;
          default:                /* Unknown; should never be reached. */
            terms[j] = 0.0;
            break;
        }
        ia[j] = 1;
        j++;
      }
    }

    if (gaussfit(x, y, nxy, sig, nsig, terms, ia, nsize, &chi2)) {
      wipoutput(stderr, "Trouble finding a Gaussian fit.\n");
      Free(ia);
      return 1;
    }

    /* Write fits to user variables. */
    for (j = 0; j < nsize; j++) {
      if ((pvar = wipparse(&rest)) == (char *)NULL)
        break;
      idum = (j / 3) + 1;
      if ((j % 3) == 0)
        wipoutput(stdout, "\nGaussian fit %d (Amp,Mean,FWHM) = ", idum);
      if ((j % 3) == 2)
	      wipoutput(stdout, "%G  ", terms[j]*1.665109222);   /* 2*sqrt(ln(2)) */
      else
	      wipoutput(stdout, "%G  ", terms[j]);
#ifdef MAGICFIX
      if (*pvar == MAGICFIX) pvar++;
#endif
      if (wipisnumber(pvar, &arg)) {
        /* No need to warn about this...?
        wipoutput(stdout, "Term %d was input as a number; ", j + 1);
        wipoutput(stdout, "It's return fit value is %G\n", terms[j]);
        */
        continue;                           /* Just a simple number. */
      }
      SPrintf(uservar, "%s %G", pvar, terms[j]);
      if (wipsetuser(uservar))
        j = nsize;
    }

    while (j++ < nsize) {             /* Output remaining fit terms. */
      idum = (j / 3) + 1;
      if (((j - 1) % 3) == 0)
        wipoutput(stdout, "\nGaussian fit %2d (amp,mean,fwhm) = ", idum);
      wipoutput(stdout, "%G  ", terms[j-1]);
    }

     /* If any variables exist, fill them with the chi-square value. */
    wipoutput(stdout, "\nchi2 = %G\n", chi2);
    while ((pvar = wipparse(&rest)) != (char *)NULL) {
#ifdef MAGICFIX
      if (*pvar == MAGICFIX) pvar++;
#endif
      if (wipisnumber(pvar, &arg)) {
        /* No need to warn about this...?
        wipoutput(stdout, "Chi-square term was input as a number; ");
        wipoutput(stdout, "It's return value is %G\n", chi2);
        */
        continue;                           /* Just a simple number. */
      }
      SPrintf(uservar, "%s %G", pvar, chi2);
      if (wipsetuser(uservar))
        break;
    }

    Free(ia);
    return 0;
}
