/*
	<matrix.c>
	09apr90 jm  Original code from Numerical Recipes in C.
	01dec90 jm  Modified to only allow arrays with first index = [0][0].
	26jul93 jm  Changed matrix routine to be index 1 based.
	28jun96 jm  Modified matrix routine for arbitrary based indicies.
         2dec00 pjt ansi-c; noted some leak locations via insure++ here
	            (6400408 bytes)

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
float *vector(int nx)
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
float **matrix(int offx, int nx, int offy, int ny)
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

void freevector(float *vector)
{
    Free(vector);
}

/*
 *  Frees the storage associated with a float matrix with the
 *  subscript range:
 *    m[offx..nx-1+offx][offy..ny-1+offy].
 */
void freematrix(float **matrix, int offx, int offy)
{
    Free(matrix[offx]+offy);
    Free(matrix+offx);
}
