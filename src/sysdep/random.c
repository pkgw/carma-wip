/*
	<random.c> - Code to generate random numbers (useful for testing)
	02jul96 jm  Original code adopted from Numerical Recipies.

Routines:
float wiprand ARGS(( long int *seed ));
float wipgaussdev ARGS(( long int *seed ));
*/

#include <wip.h>

/* Global variables for just this file */

#define IA       16807
#define IM  2147483647
#define AM    (1.0/IM)
#define IQ      127773
#define IR        2836
#define NTAB        32
#define NDIV (1+(IM-1)/NTAB)
#define EPS     1.2E-7
#define RNMX (1.0-EPS)

/* Code */

/*
 *  This routine returns a uniform random deviate between [0.0, 1.0).
 *  Set the seed to any negative value to initialize or reinitialize
 *  the sequence.
 */
#ifdef PROTOTYPE
float wiprand(long int *seed)
#else
float wiprand(seed)
long int *seed;
#endif /* PROTOTYPE */
{
    int j;
    long int k;
    static long int iy = 0;
    static long int iv[NTAB];
    float temp;

    if ((*seed <= 0) || (iy == 0)) {            /* (Re)initialization. */
      if (-(*seed) < 1)
        *seed = 1;
      else
        *seed = -(*seed);

      for (j = (NTAB + 7); j >= 0 ; j--) {
        k = (*seed) / IQ;
        *seed = (IA * (*seed - (k * IQ))) - (IR * k);
        if (*seed < 0) *seed += IM;
        if (j < NTAB) iv[j] = *seed;
      }

      iy = iv[0];
    }

    k = (*seed) / IQ;
    *seed = (IA * (*seed - (k * IQ))) - (IR * k);
    if (*seed < 0) *seed += IM;
    j = iy / NDIV;
    iy = iv[j];
    iv[j] = *seed;
    temp = AM * iy;
    if (temp > RNMX) temp = RNMX;

    return(temp);
}

/*
 *  This routine returns a normally distributed deviate with zero mean
 *  and unit variance.  This uses wiprand() as the source of uniform
 *  deviates.  Set the seed to any negative value to initialize or
 *  reinitialize the sequence.
 */
#ifdef PROTOTYPE
float wipgaussdev(long int *seed)
#else
float wipgaussdev(seed)
long int *seed;
#endif /* PROTOTYPE */
{
    static int iset = 0;
    static float gset;
    float fac, rsq, v1, v2;

    if (iset == 0) {               /* There is no extra deviate handy. */
      /*
       *  Pick two uniform numbers in the square extending from -1
       *  to +1 in each direction.  If they are not inside the unit
       *  circle, then try again.
       */
      do {
        v1 = (2.0 * wiprand(seed)) - 1.0;
        v2 = (2.0 * wiprand(seed)) - 1.0;
        rsq = (v1 * v1) + (v2 * v2);
      } while ((rsq >= 1.0) || (rsq == 0.0));
      /*
       *  Make the Box-Muller transformation to get two normal deviates.
       *  Return one and save the other for the next time.
       */
      fac = SQRT(-2.0 * LOG(rsq) / rsq);
      gset = v1 * fac;
      iset = 1;                                       /* Set the flag. */
      return(v2 * fac);
    } else {                   /* There is an extra deviate available. */
      iset = 0;                                     /* Reset the flag. */
      return(gset);
    }
}
