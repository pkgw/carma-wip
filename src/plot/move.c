/*
	<move.c>
	10apr91 jm  Original code.

Routines:
void wipmove ARGS(( float x, float y ));
void wipdraw ARGS(( float x, float y ));
void wipgetcxy ARGS(( float *cx, float *cy ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
void wipmove(float x, float y)
#else
void wipmove(x, y)
float x, y;
#endif /* PROTOTYPE */
{
      cpgmove(x, y);
      (void)wipsetvar("cx", (double)x);
      (void)wipsetvar("cy", (double)y);
      return;
}

#ifdef PROTOTYPE
void wipdraw(float x, float y)
#else
void wipdraw(x, y)
float x, y;
#endif /* PROTOTYPE */
{
      cpgdraw(x, y);
      (void)wipsetvar("cx", (double)x);
      (void)wipsetvar("cy", (double)y);
      return;
}

#ifdef PROTOTYPE
void wipgetcxy(float *cx, float *cy)
#else
void wipgetcxy(cx, cy)
float *cx, *cy;
#endif /* PROTOTYPE */
{
      double arg;
      LOGICAL error;

      arg = wipgetvar("cx", &error);
      *cx = (error == TRUE) ? 0.0 : arg;
      arg = wipgetvar("cy", &error);
      *cy = (error == TRUE) ? 0.0 : arg;
      return;
}
