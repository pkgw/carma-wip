/*
	<decode.c>
	03may91 jm  Original code.
	14jul91 jm  Modified docoding section so as to not conflict
		    with PGPLOT.  I have made the substitution of `any'
		    user variable xxx possible by adding the flag \[xxx].
	27jul92 jm  Modified to allow string variables.
	08dec92 jm  Modified to strip leading blanks from string
		    returned from wipfpfmt().
	12dec94 jm  Changed output array from int to char.

Routines:
void wipdecode ARGS(( Const char *string, char *outstr, size_t maxout ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
void wipdecode(Const char *string, char *outstr, size_t maxout)
#else
void wipdecode(string, outstr, maxout)
Const char *string;
char *outstr;
size_t maxout;
#endif /* PROTOTYPE */
{
    char *ptr, *tmptr, *savptr;
    char ch;
    char tempstr[BUFSIZ], variable[BUFSIZ];
    int nsig, nopen;
    double arg;
    LOGICAL error;

    ptr = (char *)string;
    if (Strchr(string, ESC) != (char *)NULL) {
      arg = wipgetvar("nsig", &error);
      nsig = NINT(arg);        /* Tells how to format a user variable. */
      tmptr = tempstr;
      while (*ptr) {
        if (*ptr != ESC) {
          *tmptr++ = *ptr++;               /* Just copy the character. */
          continue;                         /* Get the next character. */
        }            /* At this point an ESC character has been found. */
        ch = *ptr++;                        /* save the ESC character. */
        if (*ptr != '[') {  /* No user variable here, so just store... */
          *tmptr++ = ch;        /* ...the two characters and continue. */
          *tmptr++ = *ptr++;           /* This is so "\\" gets passed. */
          continue;                         /* Get the next character. */
        }        /* At this point a user variable flag has been found. */
        ptr++;                          /* Increment ptr past the '['. */
        savptr = variable;       /* Set up user variable name pointer. */
        nopen = 1;              /* Initialize the number of open '['s. */
        while ((*ptr) && (nopen)) {         /* Get user variable name. */
          if (*ptr == '[') nopen++;
          if (*ptr == ']') nopen--;
          if (nopen) *savptr++ = *ptr++;
        }
        if (*ptr != Null) ptr++;                 /* Skip over last ']' */
        *savptr = Null;        /* Skip last ']'; add terminating Null. */
        if (wipisstring(variable)) {          /* User string variable. */
          savptr = wipgetstring(variable);    /* Find string variable. */
          if (savptr == (char *)NULL) savptr = "";     /* Error check. */
        } else {                            /* Standard user variable. */
          arg = wipevaluate(variable, &error);  /* Find user variable. */
          savptr = wipfpfmt(arg, nsig);           /* Format the value. */
        }
        while ((*savptr != Null) && (isspace(*savptr)))
          savptr++;                       /* Strip off leading blanks. */
        while (*savptr) *tmptr++ = *savptr++;    /* Include the value. */
      }                                 /* End of "while (*ptr)" loop. */
      *tmptr = Null;              /* Terminate the string with a Null. */
      ptr = tempstr;        /* Assign the work pointer to this string. */
    }                           /* End of "(domacs == TRUE)" if block. */

    if (Strlen(ptr) >= maxout)
      wipoutput(stderr, "Decoded string overflows output string size.\n");

    (void)Strncpy(outstr, ptr, maxout);
    outstr[maxout-1] = Null;       /* Make sure it is Null terminated. */

    return;
}
