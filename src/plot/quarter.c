/*
	<quarter.c>
	25apr91 jm  Original code.
	28oct91 jm  Modified for new image variable.
	02aug92 jm  Modified to return status (void -> int) rather than
		    using a passed LOGICAL pointer.
	10nov93 jm  Modified image variable to new opaque pointer type.

Routines:
int wipquarter ARGS(( int quadrant ));
*/

#include "wip.h"

/* Global Variables needed just for this file */

/* Code */

/* Returns 0 on success; 1 on error. */
#ifdef PROTOTYPE
int wipquarter(int quadrant)
#else
int wipquarter(quadrant)
int quadrant;
#endif /* PROTOTYPE */
{
    Void *curimage;
    int nx, ny;
    int x1, x2, y1, y2;

    if (((curimage = wipimcur("curimage")) == (char *)NULL) ||
        (wipimagexists(curimage) == 0)) {
      wipoutput(stderr, "You must specify an image first!\n");
      return(1);
    }

    wipimagenxy(curimage, &nx, &ny);      /* Get the image dimensions. */
    switch (quadrant) {
      case  1: x1 = 1;    x2 = nx/2;   y1 = 1;    y2 = ny/2;   break;
      case  2: x1 = nx/2; x2 = nx;     y1 = 1;    y2 = ny/2;   break;
      case  3: x1 = 1;    x2 = nx/2;   y1 = ny/2; y2 = ny;     break;
      case  4: x1 = nx/2; x2 = nx;     y1 = ny/2; y2 = ny;     break;
      default: x1 = nx/4; x2 = 3*nx/4; y1 = ny/4; y2 = 3*ny/4; break;
    }

    wipsetsub(x1, x2, y1, y2);
    return(0);
}
