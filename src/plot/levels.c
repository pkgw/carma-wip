/*
	<levels.c>
	11aug90 jm  Original code.
	20feb92 jm  Added wipscalevels() and wipsetslev() routines.
	02aug92 jm  Fixed a bug in wipscalevels() and cleaned up code a bit.
		    Also modified wipsetslev() to return status int.
	12dec94 jm  Removed task wipsetslev().

Routines:
int wiplevels ARGS(( char *rest, float levels[], int maxlev ));
int wipautolevs ARGS(( char *rest, float levels[], int maxlev, float pmin, float pmax ));
int wipscalevels ARGS(( Const char *stype, float slev, float pmin, float pmax, float levels[], int nlev ));
*/

#include "wip.h"

/* Global Variables needed just for this file */

/* Code */

/*  Returns the number of levels parsed; 0 on error. */
#ifdef PROTOTYPE
int wiplevels(char *rest, float levels[], int maxlev)
#else
int wiplevels(rest, levels, maxlev)
char *rest;
float levels[];
int maxlev;
#endif /* PROTOTYPE */
{
    char *par;
    int nlevel;
    double arg;

    nlevel = 0;
    while (((par = wipparse(&rest)) != (char *)NULL) && (nlevel < maxlev)) {
      if (wiparguments(&par, 1, &arg) != 1) return(0);
      levels[nlevel++] = arg;
    }

    if (par != (char *)NULL)  /* (nlevel == maxlev) and more to parse. */
      wipoutput(stderr, "Sorry, no more than %d levels allowed.\n", maxlev);

    return(nlevel);
}

/*  Returns the number of levels generated; 0 on error. */
#ifdef PROTOTYPE
int wipautolevs(char *rest, float levels[], int maxlev, float pmin, float pmax)
#else
int wipautolevs(rest, levels, maxlev, pmin, pmax)
char *rest;
float levels[];
int maxlev;
float pmin, pmax;
#endif /* PROTOTYPE */
{
    char *par;
    char levtype[30];
    register int j;
    int iwant, nlevel;
    double arg, slope;

    if ((par = wipparse(&rest)) == (char *)NULL)
      return(0);
    if (wiparguments(&par, 1, &arg) != 1)
      return(0);

    iwant = INT(arg);
    if (iwant < 1) {
      wipoutput(stderr, "Insufficient number of levels: %d.\n", iwant);
      return(0);
    }

    if (iwant > maxlev) {
      wipoutput(stderr, "Too many levels: %d.\n", iwant);
      wipoutput(stderr, "Number truncated to %d levels.\n", maxlev);
      iwant = maxlev;
    }

    (void)Strcpy(levtype, "linear");   /* Set up a default level type. */
    if ((par = wipparse(&rest)) != (char *)NULL)
      (void)Strcpy(levtype, par);            /* Use the user's choice. */
    wiplower(levtype);                  /* Make sure it is lower case. */

    /* If the user desires they can override the image min/max values. */

    if ((par = wipparse(&rest)) != (char *)NULL) {
      if (wiparguments(&par, 1, &arg) == 1) {
        pmin = arg;
        if ((par = wipparse(&rest)) != (char *)NULL) {
          if (wiparguments(&par, 1, &arg) == 1) {
            pmax = arg;
          }
        }
      }
    }
      
    if (pmin == pmax) {
      wipoutput(stderr, "You must specify a contour level MIN and MAX.\n");
      wipoutput(stderr, "MAX set to (MIN + 1).\n");
      pmax = pmin + 1;
    }

    nlevel = 0;
    if (iwant == 1) {
      levels[nlevel++] = pmin;
    } else if (Strncmp(levtype, "log", 3) == 0) {
      slope = -1;                   /* Set to a wrong value initially. */
      if (pmin != 0) slope = pmax / pmin;
      if (slope <= 0) {                       /* Test for correctness. */
        wipoutput(stderr,
          "Illegal max/min values for log levels: %f %f.\n", pmax, pmin);
        return(0);
      }
      slope = LOG10(slope) / (iwant - 1.0);
      for (j = 0; j < iwant; j++) {
        levels[nlevel++] = pmin * POW(10.0, (slope * j));
      }
    } else if (Strncmp(levtype, "lin", 3) == 0) {
      slope = (pmax - pmin) / (iwant - 1.0);
      for (j = 0; j < iwant; j++) {
        levels[nlevel++] = pmin + (slope * j);
      }
    } else {
      wipoutput(stderr, "Sorry, unknown autolevel option: %s.\n", levtype);
      return(0);
    }
    return(nlevel);
}

/*
 *  Returns the number of contour level array elements modified.
 *  If (nlev <= 0), the array is generated; if (nlev > 0), "slev" and
 *  "stype" are used to modify the array.  If (slev == 1.0), (nlev > 0),
 *  and ("stype" is "absolute"), then no changes will be applied to the
 *  input contour level array.
 */
#ifdef PROTOTYPE
int wipscalevels(Const char *stype, float slev, float pmin, float pmax, float levels[], int nlev)
#else
int wipscalevels(stype, slev, pmin, pmax, levels, nlev)
Const char *stype;
float slev, pmin, pmax;
float levels[];
int nlev;
#endif /* PROTOTYPE */
{
    char *leveltype;
    int i, ilev;
    float off;

    if ((leveltype = wipleading(stype)) == (char *)NULL)
      leveltype = "abs";

    ilev = 0;
    if (nlev <= 0) {
      if ((pmax > 0) && (pmin < 0)) {
        slev = MAX( ABS(pmin), ABS(pmax) ) / 8.0;
        nlev = ABS(pmin) / slev;
        for (i = -nlev; i < 0; i++)
          levels[ilev++] = i * slev;
        nlev = pmax / slev;
        for (i = 1; i <= nlev; i++)
          levels[ilev++] = i * slev;
        nlev = ilev - 1;
        slev = 1.0;
      } else {
        nlev = 10;
        off = 0.05 * (pmax - pmin);
        pmax -= off;
        pmin += off;
        slev = (pmax - pmin) / (nlev - 1.0);
        for (i = 0; i < nlev; i++)
          levels[ilev++] = pmin + (i * slev);
        slev = 1.0;
      }
    } else {
      if (slev == 0.0) slev = 1.0;
      if ((*leveltype == 'p') || (*leveltype == 'P'))
        slev *= (pmax / 100.0);
    }

    if (slev != 1.0) {
      for (i = 0; i < nlev; i++)
        levels[i] *= slev;
    }

    return(nlev);
}
