/*
	<putlab.c>
	11apr90 jm  Original code.
	02nov91 jm  Modified for stack variables and increased
		    fixputlabel() string sizes to STRINGSIZE.
	02aug92 jm  Modified wipmtext() to return status (void -> int)
		    rather than using a passed LOGICAL pointer.
	16aug92 jm  Modified wipputlabel() to return pointer to a static
		    char array rather than a local (fixes a scope error).
	12dec94 jm  Modified arguments to wipmtext() to make wipexecute()
		    parse the arguments.

Routines:
void wipputlabel ARGS(( Const char *line, double xjust ));
char *fixputlabel ARGS(( Const char *commname, char *rest, LOGICAL save ));
int wipmtext ARGS(( char *side, float disp, float coord, float just, char *line ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  Puts a label justified according to xjust [0,1]:
 *    xjust =   0: left justified
 *          =   1: right justified
 *          = 0.5: centered
 *          = 0.x: 0.x*100% right justified
 */
#ifdef PROTOTYPE
void wipputlabel(Const char *line, double xjust)
#else
void wipputlabel(line, xjust)
Const char *line;
double xjust;
#endif /* PROTOTYPE */
{
    char newstr[STRINGSIZE];
    float xlen, ylen;
    float px, py, angle, justify;
    double arg;
    LOGICAL error;

    wipgetcxy(&px, &py);
    arg = wipgetvar("angle", &error);
    angle = (error == TRUE) ? 0.0 : arg;

    wipdecode(line, newstr, STRINGSIZE);

    justify = xjust;
    cpgptxt(px, py, angle, justify, newstr);

    cpglen(4, newstr, &xlen, &ylen);    /* World coordinates returned. */
    angle *= RPDEG;
    xlen *= (1 - xjust);
    ylen *= (1 - xjust);
    px += (xlen * COS(angle));
    py += (ylen * SIN(angle));
    wipmove(px, py);
}

/*
 *  Returns a pointer to the corrected "rest" of command string.
 *  the input is from command "putlabel x str" without the command
 *  name "putlabel".  If (x < 0), the cursor routine is called and
 *  the position is loaded with a "move" command.  On return, the
 *  pointer contains the string "str" preceeded by the value ABS(x).
 */
#ifdef PROTOTYPE
char *fixputlabel(Const char *commname, char *rest, LOGICAL save)
#else
char *fixputlabel(commname, rest, save)
Const char *commname;
char *rest;
LOGICAL save;
#endif /* PROTOTYPE */
{
    char *par, *ptr;
    char outputstr[STRINGSIZE];
    static char string[STRINGSIZE];
    float locat, argx, argy;
    double darg;

    if (Strcmp(commname, "putlabel") != 0)         /* Only "putlabel". */
      return(rest);

    ptr = Strcpy(string, rest);                  /* Make a local copy. */
    if ((par = wipparse(&ptr)) == (char *)NULL)      /* Nothing input. */
      return(rest);
    if (wiparguments(&par, 1, &darg) != 1)             /* Input error. */
      return(rest);
    if (darg >= 0)                /* Do nothing if "darg" is positive. */
      return(rest);
    locat = ABS(darg);

    (void)Strcpy(outputstr, ptr);       /* Save the "putlabel" string. */

    wipgetcxy(&argx, &argy);          /* Get the current x/y position. */
    wipcursor(&argx, &argy, FALSE);        /* Get the cursor position. */

    if (save == TRUE) { /* If needed, save a generated "move" command. */
      SPrintf(string, "move %f %f\n", argx, argy);     /* The command. */
      wipsaveline(BUFFER, string);
    }

    wipmove(argx, argy);             /* Go to the new (x, y) position. */

    /* Generate and return a new "rest of the command" string. */

    SPrintf(string, "%f %s", locat, outputstr);
    return(string);
}

/*  Returns 0 on success; 1 on error. */
#ifdef PROTOTYPE
int wipmtext(char *side, float disp, float coord, float just, char *line)
#else
int wipmtext(side, disp, coord, just, line)
char *side;
float disp;
float coord;
float just;
char *line;
#endif /* PROTOTYPE */
{
    char newstr[STRINGSIZE];

    if (wiplenc(side) < 1) {       /* Also strips off trailing blanks. */
      wipoutput(stderr, "Trouble interpreting side: [%s]\n", side);
      return(1);
    }

    if (wiplenc(line) < 1) {
      wipoutput(stderr, "Trouble interpreting text: [%s]\n", line);
      return(1);
    }

    wipdecode(line, newstr, STRINGSIZE);
    cpgmtxt(side, disp, coord, just, newstr);

    return(0);
}
