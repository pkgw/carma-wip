/*
	<polyfit.c> -- Code to parse and perform various polynomial fits.
	28jun96 jm  Original code modified from Numerical Recipes.
	12may97 jm  Zeroed the TOL definition (originally 1.0E-5).
		    It was causing elements of the w[] array to be zeroed
		    unnecessarily (and causing incorrect results).

Routines:
static void fleg ARGS(( float x, float pl[], int nl ));
static void fpoly ARGS(( float x, float p[], int np ));
static float pythag ARGS(( float a, float b ));
static int svbksb ARGS(( float **u, float w[], float **v, int m, int n, \
    float b[], float x[] ));
static int svdcmp ARGS(( float **a, int m, int n, float w[], float **v ));
static int svdfit ARGS(( float x[], float y[], int nxy, \
    float sig[], int nsig, float a[], int ma, float **u, float **v, \
    float w[], float *chisq, \
    void (*funcs) ARGS(( float, float [], int )) ));
static int svdvar ARGS(( float **v, int ma, float w[], float **cvm ));
static int polyfit ARGS(( float x[], float y[], int nxy, \
    float sig[], int nsig, int type, int npoly, float poly[], float err[], \
    float *chisq ));

int wippolyfit ARGS(( char *rest, float x[], float y[], int nxy, \
    float sig[], int nsig, int npoly, float poly[] ));
*/

#include "wip.h"

#define SIGN(a,b) ((b) >= 0.0 ? ABS(a) : -ABS(a))

/* Global variables for just this file */

/* Code */

/*
 *  Fitting routine for an expansion with nl Legendre polynomials
 *  (pl[0..nl-1]) evaluated using the recurrence relation:
 *    (n + 1)P_{n + 1}(x) = (2n + 1)xP_{n}(x) - nP_{n - 1}(x)
 */
#ifdef PROTOTYPE
static void fleg(float x, float pl[], int nl)
#else
static void fleg(x, pl, nl)
float x;
float pl[];
int nl;
#endif /* PROTOTYPE */
{
    int j;
    float twox, f1, f2, d;

    pl[0] = 1.0;
    pl[1] = x;
    if (nl > 2) {
      twox = 2.0 * x;
      f2 = x;
      d = 1.0;
      for (j = 2; j < nl; j++) {
          f1 = d++;
          f2 += twox;
          pl[j] = ((f2 * pl[j - 1]) - (f1 * pl[j - 2])) / d;
      }
    }
}

/*
 *  Fitting routine for a polynomial (p[0..np-1]) of degree (np-1)
 *  with np coefficients. 
 */
#ifdef PROTOTYPE
static void fpoly(float x, float p[], int np)
#else
static void fpoly(x, p, np)
float x;
float p[];
int np;
#endif /* PROTOTYPE */
{
    int j;

    p[0] = 1.0;
    for (j = 1; j < np; j++)
      p[j] = p[j - 1] * x;

    return;
}

#ifdef PROTOTYPE
static float pythag(float a, float b)
#else
static float pythag(a, b)
float a;
float b;
#endif /* PROTOTYPE */
{
    float absa, absb, value;

    absa = ABS(a);
    absb = ABS(b);

    if (absa > absb) {
      value = absb / absa;
      value = absa * SQRT(1.0 + (value * value));
    } else if (absb == 0.0) {
      value = 0.0;
    } else {
      value = absa / absb;
      value = absb * SQRT(1.0 + (value * value));
    }

    return(value);
}

/*
 *  This solves A x X = B for a vector X, where A is specified by the
 *  arrays U, W, V as returned by svdcmp().  M and N are the logical
 *  dimensions of A, and will be equal for square matrices.  MP and NP
 *  are the physical dimensions of A.  B is the input right-hand side.
 *  X is the output solution vector.  No input quantities are destroyed,
 *  so the routine may be called sequentially with different B's.
 *
 *  Returns 1 on allocation failure; 0 if successful.
 */
#ifdef PROTOTYPE
static int svbksb(float **u, float w[], float **v, int m, int n, float b[], float x[])
#else
static int svbksb(u, w, v, m, n, b, x)
float **u;     /* u[0..m-1][0..n-1] */
float w[];     /* w[0..n-1] */
float **v;     /* v[0..n-1][0..n-1] */
int m;
int n;
float b[];     /* b[0..m-1] */
float x[];     /* x[0..n-1] */
#endif /* PROTOTYPE */
{
    int i, j;
    float *tmp;
    float s;

    if ((tmp = vector(n)) == (float *)NULL) {
      wipoutput(stderr, "Trouble allocating temporary storage in svbksb().\n");
      return(1);
    }

    /* Calculate U^{T} B. */
    for (j = 0; j < n; j++) {
      s = 0.0;
      if (w[j] != 0.0) {  /* Non-zero result only if w(j) is non-zero. */
        for (i = 0; i < m; i++) s += (u[i][j] * b[i]);
        s /= w[j];
      }
      tmp[j] = s;
    }

    /* Matrix multiply by V to get the answer. */
    for (j = 0; j < n; j++) {
      s = 0.0;
      for (i = 0; i < n; i++) s += (v[j][i] * tmp[i]);
      x[j] = s;
    }

    freevector(tmp);

    return(0);
}

/*
 *  Given a matrix A, with logical dimensions M by N and physical
 *  dimensions MP by NP, this routine computes its singular value
 *  decomposition, A = U * W * V^T.  The matrix U replaces A on
 *  output.  The diagonal matrix of singular values W is output
 *  as a vector W.  The matrix V (not th4e transpose V^T) is
 *  output as V.  M must be greater or equal to N; if it is
 *  smaller, then A should be filled up to square with zero rows.
 *
 *  Returns 1 on allocation failure; 0 if successful.
 */
#ifdef PROTOTYPE
static int svdcmp(float **a, int m, int n, float w[], float **v)
#else
static int svdcmp(a, m, n, w, v)
float **a;
int m;
int n;
float w[];
float **v;
#endif /* PROTOTYPE */
{
    int flag, i, its, j, jj, k, L, nm;
    float anorm, c, f, g, h, s, scale, x, y, z;
    float *rv1;
    double zz;

    if ((rv1 = vector(n)) == (float *)NULL) {
      wipoutput(stderr, "Trouble allocating temporary storage in svdcmp().\n");
      return(1);
    }

    /* Householder reduction to bidiagonal form. */
    g = scale = anorm = 0.0;

    for (i = 0; i < n; i++) {
      L = i + 1;
      rv1[i] = scale * g;
      g = s = scale = 0.0;
      if (i < m) {
        for (k = i; k < m; k++) scale += ABS(a[k][i]);
        if (scale != 0.0) {
          for (k = i; k < m; k++) {
            a[k][i] /= scale;
            s += (a[k][i] * a[k][i]);
          }

          f = a[i][i];
          g = -SIGN(SQRT(s), f);
          h = (f * g) - s;
          a[i][i] = f - g;

          for (j = L; j < n; j++) {
            s = 0.0;
            for (k = i; k < m; k++) s += (a[k][i] * a[k][j]);
            f = s / h;
            for (k = i; k < m; k++) a[k][j] += (f * a[k][i]);
          }

          for (k = i; k < m; k++) a[k][i] *= scale;
        }
      }

      w[i] = scale * g;
      g = s = scale = 0.0;
      if (i < m && i != (n - 1)) {
        for (k = L; k < n; k++) scale += ABS(a[i][k]);
        if (scale != 0.0) {
          for (k = L; k < n; k++) {
            a[i][k] /= scale;
            s += (a[i][k] * a[i][k]);
          }
          f = a[i][L];
          g = -SIGN(SQRT(s), f);
          h = (f * g) - s;
          a[i][L] = f - g;

          for (k = L; k < n; k++) rv1[k] = a[i][k] / h;
          for (j = L; j < m; j++) {
            s = 0.0;
            for (k = L; k < n; k++) s += (a[j][k] * a[i][k]);
            for (k = L; k < n; k++) a[j][k] += (s * rv1[k]);
          }
          for (k = L; k < n; k++) a[i][k] *= scale;
        }
      }

      z = ABS(w[i]) + ABS(rv1[i]);
      anorm = MAX(anorm, z);
    }

    /* Accumulation of right-hand transformations. */
    for (i = n - 1; i >= 0; i--) {
      if (i < (n - 1)) {
        if (g != 0.0) {
          /* Double division to avoid possible underflow: */
          for (j = L; j < n; j++)
            v[j][i] = (a[i][j] / a[i][L]) / g;

          for (j = L; j < n; j++) {
            s = 0.0;
            for (k = L; k < n; k++) s += (a[i][k] * v[k][j]);
            for (k = L; k < n; k++) v[k][j] += (s * v[k][i]);
          }
        }
        for (j = L; j < n; j++) v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      L = i;
    }

    /* Accumulation of left-hand transformations. */
    for (i = MIN(m, n) - 1; i >= 0; i--) {
      L = i + 1;
      g = w[i];
      for (j = L; j < n; j++) a[i][j] = 0.0;
      if (g != 0.0) {
        g = 1.0 / g;
        for (j = L; j < n; j++) {
          s = 0.0;
          for (k = L; k < m; k++) s += (a[k][i] * a[k][j]);
          f = (s / a[i][i]) * g;
          for (k = i; k < m; k++) a[k][j] += (f * a[k][i]);
        }
        for (j = i; j < m; j++) a[j][i] *= g;
      } else for (j = i; j < m; j++) a[j][i] = 0.0;
      a[i][i]++;
    }

    /* Diagonalization of the bidiagonal form. */
    for (k = n - 1; k >= 0; k--) {       /* Loop over singular values. */
      for (its = 1; its <= 30; its++) { /* Loop over allowed iterations. */
        flag = 1;
        for (L = k; L >= 0; L--) {
          nm = L - 1;                  /* Note: rv1[0] is always zero. */
          zz = ABS(rv1[L]);
          if ((float)(zz + anorm) == anorm) {
            flag = 0;
            break;
          }
          if ((ABS(w[nm]) + anorm) == anorm) break;
        }

        if (flag) {              /* Cancellation of RV1(L), if L > 0:  */
          c = 0.0;
          s = 1.0;
          for (i = L; i <= k; i++) {
            f = s * rv1[i];
            rv1[i] = c * rv1[i];
            if ((ABS(f) + anorm) == anorm) break;
            g = w[i];
            h = pythag(f, g);
            w[i] = h;
            h = 1.0 / h;
            c = g * h;
            s = -f * h;
            for (j = 0; j < m; j++) {
              y = a[j][nm];
              z = a[j][i];
              a[j][nm] = (y * c) + (z * s);
              a[j][i] = (z * c) - (y * s);
            }
          }
        }

        z = w[k];
        if (L == k) {                                  /* Convergence. */
          if (z < 0.0) {        /* Singular value is made nonnegative. */
            w[k] = -z;
            for (j = 0; j < n; j++) v[j][k] = -v[j][k];
          }
          break;
        }

        if (its == 30) {
          wipoutput(stderr, "No convergence in 30 svdcmp() iterations\n");
          break;
        }

        /* Shift from bottom 2-by-2 minor: */
        x = w[L];
        nm = k - 1;
        y = w[nm];
        g = rv1[nm];
        h = rv1[k];
        f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
        g = pythag(f, 1.0);
        f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;

        /* Next ZR transformation: */
        c = s = 1.0;
        for (j = L; j <= nm; j++) {
          i = j + 1;
          g = rv1[i];
          y = w[i];
          h = s * g;
          g = c * g;
          z = pythag(f, h);
          rv1[j] = z;
          c = f / z;
          s = h / z;
          f = (x * c) + (g * s);
          g = (g * c) - (x * s);
          h = y * s;
          y *= c;
          for (jj = 0; jj < n; jj++) {
            x = v[jj][j];
            z = v[jj][i];
            v[jj][j] = (x * c) + (z * s);
            v[jj][i] = (z * c) - (x * s);
          }
          z = pythag(f, h);
          w[j] = z;            /* Rotation can be arbitrary if z == 0. */
          if (z != 0.0) {
            z = 1.0 / z;
            c = f * z;
            s = h * z;
          }
          f = (c * g) + (s * y);
          x = (c * y) - (s * g);
          for (jj = 0; jj < m; jj++) {
            y = a[jj][j];
            z = a[jj][i];
            a[jj][j] = (y * c) + (z * s);
            a[jj][i] = (z * c) - (y * s);
          }
        }
        rv1[L] = 0.0;
        rv1[k] = f;
        w[k] = x;
      }
    }

    freevector(rv1);

    return(0);
}

#define TOL 0 /* 1.0E-5 */

/*
 *  Returns 1 on allocation errors; 0 if successful.
 */
#ifdef PROTOTYPE
static int svdfit(float x[], float y[], int nxy, float sig[], int nsig,
    float a[], int ma, float **u, float **v, float w[], float *chisq,
    void (*funcs)(float, float [], int))
#else
static int svdfit(x, y, nxy, sig, nsig, a, ma, u, v, w, chisq, funcs)
float x[];     /* x[0..nxy-1] */
float y[];     /* y[0..nxy-1] */
int nxy;
float sig[];   /* sig[0..nsig-1] */
int nsig;
float a[];     /* a[0..ma-1] */
int ma;
float **u;     /* u[0..nxy-1][0..ma-1] */
float **v;     /* v[0..ma-1][0..ma-1] */
float w[];     /* w[0..ma-1] */
float *chisq;
void (*funcs)();
#endif /* PROTOTYPE */
{
    int i, j;
    float wmax, tmp, thresh, sum;
    float *b, *afunc;

    b = vector(nxy);
    afunc = vector(ma);
    if ((b == (float *)NULL) || (afunc == (float *)NULL)) {
      if (b != (float *)NULL) freevector(b);
      if (afunc != (float *)NULL) freevector(afunc);
      wipoutput(stderr, "Trouble allocating temporary storage in svdfit()\n");
      return(1);
    }

    /* Accumulate coefficients of the fitting matrix. */
    for (i = 0; i < nxy; i++) {
      (*funcs)(x[i], afunc, ma);
      tmp = (i < nsig) ? (1.0 / sig[i]) : 1.0 ;
      for (j = 0; j < ma; j++) u[i][j] = afunc[j] * tmp;
      b[i] = y[i] * tmp;
    }

    /* Singular value decomposition. */
    if (svdcmp(u, nxy, ma, w, v)) {
      freevector(afunc);
      freevector(b);
      return(1);
    }

    /* Edit the singular values, given TOL. */
    wmax = 0.0;
    for (j = 0; j < ma; j++)
      if (w[j] > wmax) wmax = w[j];

    thresh = TOL * wmax;
    for (j = 0; j < ma; j++)
      if (w[j] < thresh) w[j] = 0.0;

    if (svbksb(u, w, v, nxy, ma, b, a)) {
      freevector(afunc);
      freevector(b);
      return(1);
    }

    /* Evaluate chi-squared. */
    *chisq = 0.0;
    for (i = 0; i < nxy; i++) {
      (*funcs)(x[i], afunc, ma);
      sum = 0.0;
      for (j = 0; j < ma; j++) sum += (a[j] * afunc[j]);
      if (i < nsig)
        tmp = (y[i] - sum) / sig[i];
      else
        tmp = y[i] - sum;
      *chisq += (tmp * tmp);
    }

    freevector(afunc);
    freevector(b);

    return(0);
}

#undef TOL

/*
 *  Returns 1 if allocation error; 0 if successful.
 */
#ifdef PROTOTYPE
static int svdvar(float **v, int ma, float w[], float **cvm)
#else
static int svdvar(v, ma, w, cvm)
float **v;     /* v[0..ma-1][0..ma-1] */
int ma;
float w[];     /* w[0..ma-1] */
float **cvm;   /* cvm[0..ma-1][0..ma-1] */
#endif /* PROTOTYPE */
{
    int i, j, k;
    float sum;
    float *wti;

    if ((wti = vector(ma)) == (float *)NULL) {
      wipoutput(stderr, "Trouble allocating temporary storage in svdvar().\n");
      return(1);
    }

    for (i = 0; i < ma; i++) {
      wti[i] = 0.0;
      if (w[i] != 0.0) wti[i] = 1.0 / (w[i] * w[i]);
    }

    for (i = 0; i < ma; i++) {
      for (j = 0; j <= i; j++) {
        sum = 0.0;
        for (k = 0; k < ma; k++)
          sum += (v[i][k] * v[j][k] * wti[k]);
        cvm[j][i] = cvm[i][j] = sum;
      }
    }

    freevector(wti);

    return(0);
}

/*
 *  Returns 0 on success; 1 if allocation errors.
 */
#ifdef PROTOTYPE
static int polyfit(float x[], float y[], int nxy, float sig[], int nsig,
    int type, int npoly, float poly[], float err[], float *chisq)
#else
static int polyfit(x, y, nxy, sig, nsig, type, npoly, poly, err, chisq)
float x[];     /* [0..nxy-1] */
float y[];     /* [0..nxy-1] */
int nxy;
float sig[];   /* [0..nsig-1] */
int nsig;
int type;
int npoly;
float poly[];  /* [0..npoly-1] */
float err[];   /* [0..npoly-1] */
float *chisq;
#endif /* PROTOTYPE */
{
    int i, state;
    float *w;
    float **cvm, **u, **v;

    w = vector(npoly);
    cvm = matrix(0, npoly, 0, npoly);
    u = matrix(0, nxy, 0, npoly);
    v = matrix(0, npoly, 0, npoly);

    if ((w == (float  *)NULL) || (cvm == (float **)NULL) ||
        (u == (float **)NULL) || (v   == (float **)NULL)) {
      if (w   != (float  *)NULL) freevector(w);
      if (cvm != (float **)NULL) freematrix(cvm, 0, 0);
      if (u   != (float **)NULL) freematrix(u, 0, 0);
      if (v   != (float **)NULL) freematrix(v, 0, 0);
      wipoutput(stderr, "Trouble allocating storage for the Polynomial fit.\n");
      return(1);
    }

    if (type == 0) /* Basic polynomial fit. */
      state = svdfit(x, y, nxy, sig, nsig, poly, npoly, u, v, w, chisq, fpoly);
    else           /* Legendre polynomial fit. */
      state = svdfit(x, y, nxy, sig, nsig, poly, npoly, u, v, w, chisq, fleg);

    if (state == 0) {
      state = svdvar(v, npoly, w, cvm);
      if (state == 0) {
        for (i = 0; i < npoly; i++)
          err[i] = SQRT(cvm[i][i]);
      }
    }

    freematrix(v, 0, 0);
    freematrix(u, 0, 0);
    freematrix(cvm, 0, 0);
    freevector(w);

    return(state);
}

/*
 *  This performs the parsing and polynomial fitting.  The input string
 *  rest can contain suggestions for fit parameters and may also be
 *  used to store in user variables the found fit parameters.
 *  This string will be parsed by this routine!
 *  The x[] and y[] arrays contain nxy points to be fit.  The sig[]
 *  array contains nsig error bar values.  The term type determines
 *  what type of polynomial fit to perform.  A value for type of 0
 *  means a simple polynomial fit; and a value of 1 means to fit
 *  using Legendre polynomials.  The term npoly specifies the number
 *  of polynomials to fit.  The size of the work array poly[] should
 *  be as big as npoly.  It is returned with the fit coefficients.
 *
 *  This routine returns 0 if successful; 1 on allocation errors.
 */
#ifdef PROTOTYPE
int wippolyfit(char *rest, float x[], float y[], int nxy, float sig[],
    int nsig, int type, int npoly, float poly[])
#else
int wippolyfit(rest, x, y, nxy, sig, nsig, type, npoly, poly)
char *rest;
float x[];     /* [0..nxy-1] */
float y[];     /* [0..nxy-1] */
int nxy;
float sig[];   /* [0..nsig-1] */
int nsig;
int type;
int npoly;
float poly[];  /* [0..npoly-1] */
#endif /* PROTOTYPE */
{
    char *ptr, *pvar;
    char uservar[STRINGSIZE];
    int i, nitem;
    int state;
    float chisq;
    float *err;

    if (npoly < 2) {
      wipoutput(stderr, "Number of polynomials to fit [%d] is too small.\n",
        npoly);
      return(1);
    }

    if ((err = vector(npoly)) == (float  *)NULL) {
      wipoutput(stderr, "Trouble allocating storage for the Polynomial fit.\n");
      return(1);
    }

    state = polyfit(x, y, nxy, sig, nsig, type, npoly, poly, err, &chisq);

    if (state == 0) {
      if (type == 0)
        wipoutput(stdout, "Polynomial fit parameters:\n");
      else
        wipoutput(stdout, "Legendre polynomial fit parameters:\n");

      for (i = 0; i < npoly; i++)
        wipoutput(stdout, "P[%d] = %12.6f +- %10.6f\n", i, poly[i], err[i]);

      wipoutput(stdout, "Chi-squared %12.6f\n", chisq);

      ptr = rest;
      for (nitem = 0; ((pvar = wipparse(&ptr)) != (char *)NULL); nitem++) {
        i = nitem / npoly;
        switch(i) {
          case 0:
            SPrintf(uservar, "%s %G", pvar, poly[nitem]);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 1:
            SPrintf(uservar, "%s %G", pvar, err[nitem - npoly]);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            break;
          case 2:
            SPrintf(uservar, "%s %G", pvar, chisq);
            if (wipsetuser(uservar)) ptr = (char *)NULL;
            nitem = 3 * npoly;
            break;
          default:
            wipoutput(stderr, "Extra arguments ignored.\n");
            ptr = (char *)NULL;
            break;
        }
      }
    }

    freevector(err);

    return(state);
}

#ifdef TEST
/* Test driver for routine wippolyfit */
#define NPT 100
#define SPREAD 0.02
#define NPOL 5

static double a[NPOL] = {1.0, 2.0, 3.0, 4.0, 5.0};

#ifdef PROTOTYPE
static double polval(float x, int order)
#else
static double polval(x, order)
float x;
int order;
#endif /* PROTOTYPE */
{
    int j;
    double y, z;

    y = a[0];
    z = 1.0;
    for (j = 1; j < order; j++) {
      z *= x;
      y += (z * a[j]);
    }

    return(y);
}

#ifdef PROTOTYPE
int main(void)
#else
int main()
#endif /* PROTOTYPE */
{
    char *ptr = "";
    int i, state;
    long int seed;
    float *x, *y, *sig, *poly;

    seed = (-911);
    x = vector(NPT);
    y = vector(NPT);
    sig = vector(NPT);
    poly = vector(NPOL);

    for (i = 0; i < NPT; i++) {
      x[i] = 0.02 * (i + 1);
      y[i] = polval(x[i], NPOL);
      y[i] *= (1.0 + (SPREAD * wipgaussdev(&seed)));
      sig[i] = y[i] * SPREAD;
    }

    state = wippolyfit(ptr, x, y, NPT, sig, NPT, 0, NPOL, poly);
    wipoutput(stdout, "Polynomial fit returned error state [%d].\n", state);

    state = wippolyfit(ptr, x, y, NPT, sig, NPT, 1, NPOL, poly);
    wipoutput(stdout, "Legendre polynomial fit returned error state [%d].\n",
      state);

    freevector(poly);
    freevector(sig);
    freevector(y);
    freevector(x);

    return(0);
}
#endif /* TEST */
