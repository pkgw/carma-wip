/*
	<parse.c>
	17jul90 jm  Original code.
	27jul90 jm  Parse and arg corrected.
	02nov91 jm  Removed wipnumarg() in favor of new routine wipevaluate().
	20feb92 jm  Changed wiplenc to change trailing WHITE to Null's.
	28jul92 jm  Added wipnewstring() routine.
	11sep92 jm  Added test for empty (NULL) strings.

Routines:
char *wipparse ARGS(( char **line ));
int wipcountwords ARGS(( Const char *line ));
void wiplower ARGS(( char *s ));
void wipupper ARGS(( char *s ));
char *wipleading ARGS(( Const char *line ));
int wiplenc ARGS(( char *c ));
char *wipnewstring ARGS(( Const char *string ));
int wiparguments ARGS(( char **rest, int n, double arg[] ));
*/

#include "wip.h"

/* Global variables for just this file */

/* Code */

#ifdef PROTOTYPE
char *wipparse(char **line)
#else
char *wipparse(line)
char **line;
#endif /* PROTOTYPE */
{
    char *s, *par;

    s = *line;
    if ((s == (char *)NULL) || (*s == Null))           /* Empty string. */
      return((char *)NULL);

    while ((*s != Null) && (WHITE(*s))) s++;    /* Skip leading blanks. */
    if (*s == Null) return((char *)NULL);     /* Nothing left to parse. */
    if (*s == '#') return((char *)NULL);             /* A Comment line. */

    par = s;                                           /* Found a word. */
    while ((*s != Null) && (!WHITE(*s)))    /* Extend over entire word. */
      s++;
    if (*s != Null) *s++ = Null;     /* Don't increment if at EOString. */
    *line = s;                                  /* Reset input pointer. */
    return(par);                                  /* Return found word. */
}

#ifdef PROTOTYPE
int wipcountwords(Const char *line)
#else
int wipcountwords(line)
Const char *line;
#endif /* PROTOTYPE */
{
    register int n = 0;
    char *s;
    char save[BUFSIZ];

    s = Strcpy(save, line);   /* Make a temporary copy of input string. */
    while (wipparse(&s) != (char *)NULL) n++;       /* Count the words. */
    return(n);
}

#ifdef PROTOTYPE
void wiplower(char *s)
#else
void wiplower(s)
char *s;
#endif /* PROTOTYPE */
{
    register char j;

    while ((j = *s) != Null)
      *s++ = (isupper((int)j) ? (char)tolower((int)j) : j);
}

#ifdef PROTOTYPE
void wipupper(char *s)
#else
void wipupper(s)
char *s;
#endif /* PROTOTYPE */
{
    register char j;

    while ((j = *s) != Null)
      *s++ = (islower((int)j) ? (char)toupper((int)j) : j);
}

#ifdef PROTOTYPE
char *wipleading(Const char *line)
#else
char *wipleading(line)
Const char *line;
#endif /* PROTOTYPE */
{
    register char *s;

    s = (char *)line;
    if ((s == (char *)NULL) || (*s == Null))           /* Empty string. */
      return((char *)NULL);

    while ((*s != Null) && (WHITE(*s))) s++;    /* Skip leading blanks. */
    if (*s == Null) return((char *)NULL);      /* Nothing left to skip. */
    if (*s == '#') return((char *)NULL);             /* A Comment line. */

    return((char *)s);
}

#ifdef PROTOTYPE
int wiplenc(char *c)
#else
int wiplenc(c)
char *c;
#endif /* PROTOTYPE */
{
    register char *s;

    if ((c == (char *)NULL) || (*c == Null))           /* Empty string. */
      return(0);

    s = (char *)(c) + Strlen(c) - 1;          /* Start at the EOString. */
    while ((s >= c) && (WHITE(*s))) *s-- = Null;       /* Remove WHITE. */

    return((int)(s - c + 1));  /* Return # of chars w/o trailing WHITE. */
}

/*
 *  Copies an instance of a string.  You must release this memory with Free()!
 *  Returns a NULL pointer if the input string is empty or on an error.
 */
#ifdef PROTOTYPE
char *wipnewstring(Const char *string)
#else
char *wipnewstring(string)
Const char *string;
#endif /* PROTOTYPE */
{
    char *newstr;
    size_t strsize;

    if (((char *)string == (char *)NULL) || (*string == Null))
      return((char *)NULL);

    strsize = Strlen(string) + 1;  /* Include room for the final Null. */
    if (strsize <= 1)
      return((char *)NULL);

    if ((newstr = (char *)Malloc(strsize * sizeof(char))) == (char *)NULL) {
      wipoutput(stderr, "Could not allocate memory to make a copy of [%s].\n",
        string);
      return((char *)NULL);
    }

    (void)Strcpy(newstr, string);

    return(newstr);
}

#ifdef PROTOTYPE
int wiparguments(char **rest, int maxarg, double arg[])
#else
int wiparguments(rest, maxarg, arg)
char **rest;
int maxarg;
double arg[];
#endif /* PROTOTYPE */
{
    char *s;
    register int j;
    double val;
    LOGICAL error;

    j = 0;
    while ((j < maxarg) && ((s = wipparse(rest)) != (char *)NULL)) {
      val = wipevaluate(s, &error);
      if (error == TRUE) {
        wipoutput(stderr, "Error in numerical converson of %s.\n", s);
        break;
      }
      arg[j++] = val;
    }

    if (j < maxarg)
      wipoutput(stderr,
        "Insufficient number of arguments provided. %d required.\n", maxarg);

    return(j);
}
