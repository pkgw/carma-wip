/*
	<maxecute.c>
	31jul90 jm  Original code.
	20oct90 jm  Changed return string to static in domacsubs().
	            Also, made a local copy of rest in wipmaxecute().
	04may91 jm  Added a check for putlabel, insert, define, and
	            playback; and added a check for '$' in domacsubs().
	19nov91 jm  Removed static variables (dependency on level).
	            This allows me to take full advantage of the
	            iterative ability of C.  Changed the call sequence
	            to domacsubs().  Also moved LOGICAL error from
	            call to wipmaxecute() and wipexecute(); they now
	            return an integer=0 if okay; 1 on any error.
	02aug92 jm  Moved static code to top of file.
	16sep92 jm  Modified ifxecute to allow "if" commands to call "loop".
	            Also cleaned up code and added a few comments.

Routines:
static char *domacsubs ARGS(( char *string, Const char *line, char *macarg[] ));
int wipmaxecute ARGS(( COMMAND *comm, int count, Const char *rest ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */
/* Parameter STRINGSIZE defined in wip.h. */

#define MAXARGS 10

/* Code */

/*
 * This routine takes the current input command line "line" and
 * searches for any possible macro declarations.  These are
 * present in the line as "$#" where '#' is a single digit
 * number from 0 to 9.  If a macro argument is found, it is
 * either substituted with the value on the same line as the
 * macro call (if it was present) or silently replaced with an
 * empty string.
 *
 * The user order of macro substitutions runs from the 1st to 9th
 * and then the 0th value.  Internally, the array runs from 0 to 9.
 * Thus, when a macro argument is found, it's integer value
 * will need to be shifted to account for the difference.  This
 * is done with a simple mod operation:
 * Internal order is: $0, $1, ..., $8, $9;
 * User sees order as: $1, $2, ..., $9, $0.
 *
 * This routine uses the input string "string" to copy the argument
 * substituted line into, so make sure it is large enough to hold
 * everything!  Also, this function returns a pointer to "string"
 * in the same fashion as strcpy().  There is no check to make sure
 * that the size of macarg[] is sufficient to cover all input
 * arguments.  This is a quiet assumption.  Also, macarg should
 * be declared "Const char *macarg[]" but because it is a pointer to
 * an open array, I have trouble doing this.
 *
 */
#ifdef PROTOTYPE
static char *domacsubs(char *string, Const char *line, char *macarg[])
#else
static char *domacsubs(string, line, macarg)
char *string;
Const char *line;
char *macarg[];
#endif /* PROTOTYPE */
{
    register int j;
    register char *s, *ptr, *arg;

    if (Strchr(line, '$') == (char *)NULL) {   /* No arguments to sub. */
      (void)Strcpy(string, line);     /* Simply return a copy of line. */
    } else {
      s = (char *)line;
      ptr = string;
      while (*s) {                      /* Parse through input string. */
        if (*s != '$') {                /* If not special character... */
          *ptr++ = *s++;                /* ...copy regular characters; */
        } else {                /* otherwise, get the argument number. */
          j = (*(s+1)) - '0';      /* Convert next char to an integer. */
          if ((j < 0) || (j > 9)) {       /* Not a macro substitution. */
            *ptr++ = *s++;                      /* Just a regular '$'. */
          } else {
            j = (j + 9) % 10;       /* This is where order is shifted. */
            /*
             *  If the macro argument is not present, then quietly
             *  skip over the request.  If it is present (macarg[j]
             *  != Null), then substitute the argument value now.
             */
            if (macarg[j] != Null) {     /* If a value exists, then... */
              arg = (char *)macarg[j];  /* ...substitute the argument. */
              while (*arg)
                *ptr++ = *arg++;
            }
            s++;                                 /* Skip over the '$'. */
            s++;                                 /* Skip over the '#'. */
          }                           /* If (0 <= j <= 9) conditional. */
        }                               /* If (*s != '$') conditional. */
      }                                            /* While (*s) loop. */
      *ptr = Null;                   /* Make sure it ends with a Null. */
    }                               /* if (!Strchr(line, '$')) branch. */

    return(string);
}

/*  Returns 0 if no error; 1 otherwise. */
#ifdef PROTOTYPE
int wipmaxecute(COMMAND *comm, int count, Const char *rest)
#else
int wipmaxecute(comm, count, rest)
COMMAND *comm;
int count;
Const char *rest;
#endif /* PROTOTYPE */
{
    char *ptr;
    char *macarg[MAXARGS];             /* Pointers to macro arguments. */
    char restcopy[STRINGSIZE];          /* Private copy of input rest. */
    char substring[STRINGSIZE]; /* Command with arguments substituted. */
    int j;
    register int nloop;
    COMMAND *vb;
    register PCMACRO *pc;

    nloop = count;
    for (j = 0; j < MAXARGS; j++)
      macarg[j] = Null;   /* Initialize the pointers to the arguments. */

    ptr = Strcpy(restcopy, rest);   /* Make a local copy of the input. */
    for (j = 0; j < MAXARGS; j++)          /* Get the macro arguments. */
      if ((macarg[j] = wipparse(&ptr)) == (char *)NULL)
        break;

    for (nloop = 0; nloop < count; nloop++) {  /* Execute repeatively. */
      for (pc = comm->pcmac; pc != (PCMACRO *)NULL; pc = pc->next) {
        ptr = domacsubs(substring, pc->line, macarg);
        if ((vb = wipinterpret(&ptr)) == (COMMAND *)NULL) {
          wipoutput(stderr, "Error interpreting macro line: [%s]\n", pc->line);
          goto INPERR;
        }

        j = 1;                       /* Set up a temporary loop value. */
        if (Strcmp("loop", vb->name) == 0) {
          if ((vb = wiploopxecute(&ptr, &j)) == (COMMAND *)NULL) {
            goto INPERR;
          }
        } else if (Strcmp("if", vb->name) == 0) {
          if ((vb = wipifxecute(&ptr)) == (COMMAND *)NULL) goto INPERR;
          if (vb == ENDIF) continue;
          if (Strcmp("end", vb->name) == 0) break;
          if (Strcmp("loop", vb->name) == 0) {
            if ((vb = wiploopxecute(&ptr, &j)) == (COMMAND *)NULL) goto INPERR;
          }
        }

        if (Strcmp("define", vb->name) == 0) {
          wipoutput(stderr, "DEFINE not allowed within a macro.\n");
          goto INPERR;
        }

        if (Strcmp("insert", vb->name) == 0) {
          wipoutput(stderr, "INSERT not allowed within a macro.\n");
          goto INPERR;
        }

        if (Strcmp("playback", vb->name) == 0) {
          wipoutput(stderr, "PLAYBACK not allowed within a macro.\n");
          goto INPERR;
        }

        if (Strcmp("putlabel", vb->name) == 0)
          ptr = fixputlabel(vb->name, ptr, FALSE);

        if (Strcmp("cursor", vb->name) == 0)
          wipfixcurs(vb->name, ptr, FALSE);

        if (vb->macro == TRUE) {
          if (wipmaxecute(vb, j, ptr)) goto INPERR;
        } else {
          if (wipexecute(vb->name, ptr)) goto INPERR;
        }

      }                     /* End of for(pc != (PCMACRO *)NULL) loop. */
    }                               /* End of for(nloop < count) loop. */

    return(0);

INPERR:
    return(1);
}
