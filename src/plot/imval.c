/*
	<imval.c>
	27mar91 jm  Original code.
	26feb92 jm  Added radecfmt code.
        14aug92 jm  Modified wipradecfmt() to return pointer to a static
		    duration string rather than a local scope one.

Routines:
float wipimval ARGS(( float **image, int nx, int ny, float xpos, float ypos, float tr[], LOGICAL *error ));
char *wipradecfmt ARGS(( float position ));
*/

#include "wip.h"

/* Global Variables needed just for this file */

/* Code */

#ifdef PROTOTYPE
float wipimval(float **image, int nx, int ny, float xpos, float ypos, float tr[], LOGICAL *error)
#else
float wipimval(image, nx, ny, xpos, ypos, tr, error)
float **image;
int nx, ny;
float xpos, ypos;
float tr[];
LOGICAL *error;
#endif /* PROTOTYPE */
{
      int x, y;
      float value, pos, den;

      *error = TRUE;
      value = 0.0;
      den = ((tr[1] * tr[5]) - (tr[2] * tr[4]));
      if (den != 0.0) {
        pos = (tr[5] * (xpos - tr[0])) - (tr[2] * (ypos - tr[3]));
        x = NINT(pos / den);
        pos = (tr[1] * (ypos - tr[3])) - (tr[4] * (xpos - tr[0]));
        y = NINT(pos / den);

        x--; /* Go from 1 index based to 0 based. */
        y--; /* Go from 1 index based to 0 based. */

        if ((y >= 0) && (y < ny)) {
          if ((x >= 0) && (x < nx)) {
            value = image[y][x];
            *error = FALSE;
          }
        }
      }
      return(value);
}

#ifdef PROTOTYPE
char *wipradecfmt(float position)
#else
char *wipradecfmt(position)
float position;
#endif /* PROTOTYPE */
{
    char *ptr;
    static char string[STRINGSIZE];
    int sign, nsig;
    int radec, minutes, tensofseconds;
    float seconds;
    double dnsig;
    LOGICAL error;

    dnsig = wipgetvar("nsig", &error);
    nsig = (error == TRUE) ? 2 : NINT(dnsig);

    sign = (position < 0) ? -1 : 1;
    seconds = ABS(position);
    radec = seconds / 3600.0;
    seconds -= (radec * 3600.0);
    minutes = seconds / 60.0;
    seconds -= (minutes * 60.0);
    tensofseconds = seconds / 10.0;
    radec *= sign;

    if (!tensofseconds) nsig--;
    if (INT(seconds) == 0) nsig--;
    if (nsig < 0) nsig = 0;
    ptr = wipleading(wipfpfmt(seconds, nsig));

    if (tensofseconds) {
      SPrintf(string, "%d %02d %s", radec, minutes, ptr);
    } else {
      SPrintf(string, "%d %02d 0%s", radec, minutes, ptr);
    }
    return(string);
}
