/*
	<device.c>
	14may91 jm  Original code.
        02aug92 jm  Modified wipdevice() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
        23sep94 jm  Modified wipdevice() to reset transfer function.
        12dec94 jm  Transfer function is now done by reset call.

Routines:
int wipdevice ARGS(( Const char *rest ));
void wipclose ARGS(( void ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Opens a new device and sets up some initial plot parameters.
 *  Returns 0 if new device successfully set; 1 otherwise (on error).
 */
#ifdef PROTOTYPE
int wipdevice(Const char *rest)
#else
int wipdevice(rest)
Const char *rest;
#endif /* PROTOTYPE */
{
    char *par, *ptr;
    char string[BUFSIZ];
    int nx, ny;

    par = Strcpy(string, rest);
    if ((ptr = wipparse(&par)) == (char *)NULL)
      return(1);

    if (wiplenc(ptr) < 1) {
      wipoutput(stderr, "Device name is needed to switch devices.\n");
      return(1);
    }

   /*
    *  First, treat the special case ("device = ?") so that the
    *  current device doesn't close.
    */
    if (Strcmp(ptr, "?") == 0) {
      cpgldev();
      return(0);
    }
    
    wippanel(1, 1, 1);       /* Reset panel parameters, if necessary. */

    if (cpgbeg(0, ptr, 1, 1) != 1) {
      wipoutput(stderr, "Trouble opening new device: [%s].\n", ptr);
      return(1);
    }

    if (wipsetstring("device", ptr)) {
      wipoutput(stderr, "Trouble saving new device name [%s] locally.\n", ptr);
      return(1);
    }

    cpgask(0);                        /* Turn the new page prompt off. */
    wipreset();         /* Reset everything (color, line style, etc.). */
    wiplimits();                            /* Store the world limits. */
    wipviewport();                       /* Store the viewport limits. */
    /*  If the device maximum is greater than (or equal to) 16, then
     *  set the default range of color indices available for images to
     *  be 16 up to the device maximum.  If the device maximum is less
     *  than 16, then set it so images are not possible.
     */
    cpgqcol(&nx, &ny);              /* Get the full color index range. */
    nx = 16;
    if (ny < nx) ny = 0;
    wipsetcir(nx, ny);                            /* Store it locally. */

    return(0);
}

/*  Terminates the plotting program. */
#ifdef PROTOTYPE
void wipclose(void)
#else
void wipclose()
#endif /* PROTOTYPE */
{
      cpgend();
      return;
}
