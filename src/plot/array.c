/*
	<array.c>
	02mar91 jm  Original code.
	15aug92 jm  Modified wiperrorbar() to return a status int.
	27aug92 jm  Changed wipminimax routine to wiprange to reflect
		    change to evaluating only one array instead of two.
	26jul93 jm  Added scale input to wiplogarithm() function.
	12oct95 jm  Modified wiperrorbar() to permit locations 5/6.

Routines:
void wiplogarithm ARGS(( float array[], int nxy, float scale ));
void wiprange ARGS(( int nx, float x[], float *xmin, float *xmax ));
int wiperrorbar ARGS(( int location, float x[], float y[], float err[], int nxy ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
void wiplogarithm(float array[], int nxy, float scale)
#else
void wiplogarithm(array, nxy, scale)
float array[];
int nxy;
float scale;
#endif /* PROTOTYPE */
{
    register int j;

    for (j = 0; j < nxy; j++)
      array[j] = (array[j] > 0.0) ? (scale * LOG10(array[j])) : -50.0;

    return;
}

#ifdef PROTOTYPE
void wiprange(int nx, float x[], float *xmin, float *xmax)
#else
void wiprange(nx, x, xmin, xmax)
int nx;
float x[];
float *xmin, *xmax;
#endif /* PROTOTYPE */
{
    register int j;
    float xdiff;

    if (nx < 1) {
      *xmin = 0.0;
      *xmax = 1.0;
    } else {
      *xmin = x[0];
      *xmax = *xmin;
      for (j = 1; j < nx; j++) {
        *xmin = MIN(*xmin, x[j]);
        *xmax = MAX(*xmax, x[j]);
      }
      xdiff = *xmax - *xmin;
      if (xdiff == 0.0) xdiff = ((*xmin != 0.0) ? *xmin : 1.0);
      *xmin -= (0.05 * xdiff);
      *xmax += (0.05 * xdiff);
    }
    return;
}

/*
 *  LOCATION = 1-4 for bars to quadrant j less 45 deg, eg 1 for +X
 *  Returns 0 on success; 1 on error.
 */
#ifdef PROTOTYPE
int wiperrorbar(int location, float x[], float y[], float err[], int nxy)
#else
int wiperrorbar(location, x, y, err, nxy)
int location, nxy;
float x[], y[], err[];
#endif /* PROTOTYPE */
{
    float expsiz;
    LOGICAL error;

    if (nxy < 1) return(1);

    cpgbbuf(); /* Set up buffered output. */

    expsiz = wipgetvar("expand", &error);
    if (error == TRUE) expsiz  = 1.0;
    expsiz /= 10.0;
    cpgerrb(location, nxy, x, y, err, expsiz);

    cpgebuf(); /* Finish up buffered output. */

    return(0);
}
