/*
	<points.c>
	02mar91 jm  Original code.
	27aug92 jm  Modified to include an optional color index array.
	01sep93 jm  Added buffering commands.
	13feb95 jm  Modified to only call wipcolor when they change.

Routines:
int wippoints ARGS(( int nstyle, float style[], int nxy, float x[], \
  float y[], int nc, float c[] ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  The STYLE array has two components:
 *    (1) INT(STYLE) is the Hershey symbol number identifier.
 *    (2) FRAC(STYLE) is the relative expansion (0 means 1 however).
 *  The C array can be used to specify different colors for each point.
 */
#ifdef PROTOTYPE
int wippoints(int nstyle, float style[], int nxy, float x[], float y[], int nc, float c[])
#else
int wippoints(nstyle, style, nxy, x, y, nc, c)
int nstyle, nxy, nc;
float style[], x[], y[], c[];
#endif /* PROTOTYPE */
{
    register int j;
    int symbol;
    int saveColor, lastColor, defcolor, color, cmin, cmax;
    float xpt, ypt;
    double temp, savexp, expfrac;
    LOGICAL error;

    if (nxy < 1) return(1);

    cpgbbuf(); /* Set up buffered output. */

    savexp = wipgetvar("expand", &error);
    if (error == TRUE) savexp = 1.0;
    temp = wipgetvar("color", &error);
    saveColor = (error == TRUE) ? 1 : NINT(temp);
    cpgqcol(&cmin, &cmax);

    lastColor = defcolor = saveColor;
    if ((nc > 0) && (c[0] >= cmin) && (c[0] <= cmax))
      defcolor = c[0];

    for (j = 0; j < nxy; j++) {
      temp = ((j < nstyle) ? style[j] : style[0]) + 0.001;
      symbol = INT(temp);

      expfrac = temp - symbol;
      if (expfrac < 0.01) expfrac = 1.0;
      expfrac *= savexp;
      wipexpand(expfrac);

      if ((j < nc) && (c[j] >= cmin) && (c[j] <= cmax))
        color = c[j];
      else
        color = defcolor;

      if (lastColor != color)
        wipcolor(color);

      lastColor = color;

      xpt = x[j];
      ypt = y[j];
      cpgpt(1, &xpt, &ypt, symbol);
    }

    wipmove(xpt, ypt);
    wipexpand(savexp);
    if (lastColor != saveColor)
      wipcolor(saveColor);
    cpgebuf(); /* Finish up buffered output. */

    return(0);
}
