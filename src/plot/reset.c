/*
	<reset.c>
	03may91 jm  Original code.

Routines:
void wipreset ARGS(( void ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
void wipreset(void)
#else
void wipreset()
#endif /* PROTOTYPE */
{
      float tr[6];

      tr[0] = 0.0; tr[1] = 1.0; tr[2] = 0.0;
      tr[3] = 0.0; tr[4] = 0.0; tr[5] = 1.0;

      wipcolor(1);
      wipexpand(1.0);
      wipfill(1);
      wipfont(1);
      wipltype(1);
      wiplw(1);
      wipsetbgci(-1);

      wipsetangle(0.0);
      wipsetr(tr);
      wipsetick(0.0, 0, 0.0, 0);
      wipsetitf(0);

      wippalette(0, 0);

      return;
}
