/*
	<matrix.c>
	09apr90 jm  Original code from Numerical Recipes in C.
	01dec90 jm  Modified to only allow arrays with first index = [0][0].
	26jul93 jm  Changed matrix routine to be index 1 based.
	28jun96 jm  Modified matrix routine for arbitrary based indicies.

Routines:
float *vector ARGS(( int nx ));
float **matrix ARGS(( int offx, int nx, int offy, int ny ));
void freevector ARGS(( float *vector ));
void freematrix ARGS(( float **matrix, int offx, int offy ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/* Allocate a float array with index range [0..nx-1].  */
#ifdef PROTOTYPE
float *vector(int nx)
#else
float *vector(nx)
int nx;
#endif /* PROTOTYPE */
{
    float *v;

    v = (float *)Malloc((unsigned)(nx) * sizeof(float));

    if (v == (float *)NULL)
      wipoutput(stderr, "Allocation failure in vector().\n");

    return (v);
}

/*
 *  Allocates a float matrix with subscript range:
 *    m[offx..nx-1+offx][offy..ny-1+offy].
 *  Returns a pointer to matrix on success; NULL on failure.
 */
#ifdef PROTOTYPE
float **matrix(int offx, int nx, int offy, int ny)
#else
float **matrix(offx, nx, offy, ny)
int offx;
int nx;
int offy;
int ny;
#endif /* PROTOTYPE */
{
    int i;
    float **m;

    /* Allocate pointers to rows. */
    m = (float **)Malloc((unsigned)(nx) * sizeof(float*));
    if (m == (float **)NULL) {
      wipoutput(stderr, "Allocation failure 1 in matrix().\n");
      return((float **)NULL);
    }
    m -= offx;                      /* Convert to proper x-index base. */

    /* Allocate rows and set pointers to them. */
    m[offx] = (float *)Malloc((unsigned)(nx * ny) * sizeof(float));
    if (m[offx] == (float *)NULL) {
      wipoutput(stderr, "Allocation failure 2 in matrix().\n");
      Free(m);
      return((float **)NULL);
    }
    m[offx] -= offy;                /* Convert to proper y-index base. */

    for (i = offx + 1; i < offx + nx; i++) {
      m[i] = m[i-1] + ny;
    }

    /* Return a pointer to the array of pointers to rows. */
    return(m);
}

#ifdef PROTOTYPE
void freevector(float *vector)
#else
void freevector(vector)
float *vector;
#endif /* PROTOTYPE */
{
    Free(vector);
}

/*
 *  Frees the storage associated with a float matrix with the
 *  subscript range:
 *    m[offx..nx-1+offx][offy..ny-1+offy].
 */
#ifdef PROTOTYPE
void freematrix(float **matrix, int offx, int offy)
#else
void freematrix(matrix, offx, offy)
float **matrix;
int offx;
int offy;
#endif /* PROTOTYPE */
{
    Free(matrix[offx]+offy);
    Free(matrix+offx);
}
