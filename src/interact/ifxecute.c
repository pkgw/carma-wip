/*
 *      <ifxecute.c>
 *      ??feb88 jm  Original code.
 *      17jul90 jm  Converted to C.
 *      05nov91 jm  Major change to allow more compicated conditionals.
 *                  This is based on conversion to stack structures.
 *      01aug92 jm  Modified to return NULL cast to (COMMAND *).
 *      16sep92 jm  Modified to allow "loop" command to be called.
 *      02aug93 jm  Fixed a SEG violation in testing of right brace.
 *       9oct00 pjt no more PROTOTYPEs
 *
 *      This routine checks the conditional value of two items and
 *      executes a macro/command if the test is true.  The conditional
 *      is extracted from the input string REST as follows:
 *            rest[] = arg op arg1 mac/com p1 p2 p3 ...
 *      or
 *            rest[] = ((arg op arg1) op2 (arg2 op3 arg3)) mac/com p1 p2 p3 ...
 *      etc.
 *      where ARG's are the arguments to test with the operations OP(s).
 *      OP returns true for the following conditions:
 *            ==  arg1 equals arg2;
 *            >   arg1 greater than arg2;
 *            <   arg1 less than arg2;
 *            !=  arg1 not equal arg2;
 *            <=  arg1 less than or equal to arg2;
 *            >=  arg1 greater than or equal to arg2.
 *            &&  arg1 and arg2 are non-zero.
 *            ||  arg1 or arg2 are non-zero.
 *            ^   arg1 or arg2, but not both, are non-zero.
 *
 *      Mac/com is the macro or command to execute if the above condition
 *      tests true and p1, p2, p3, ... are parameters to pass to the macro.
 *      REST is the only input parameter and contains the substring
 *      p1 p2 p3 ... on output.  All other parameters are output and are
 *      the same as the output from MGOINTERPRET.
 *
 *  Changes:
 *      truth conditional = .false. changed from error = .true. to
 *      COMMAND = 'endif'.  This should be caught by the calling routine
 *      so as to not disturb the flow of a calling macro.  11/14/88 jm.
 *
 *      I have rehauled this code to allow multiple conditionals.  In
 *      order for this to work, the entire conditional must be bracketed
 *      by a set of parenthesis.  If this set is not present, then the
 *      routine reverts to the old scheme.  5nov91 jm.
 *
 * Routines:
 * COMMAND *wipifxecute ARGS(( char **rest ));
 */

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

COMMAND *wipifxecute(char **rest)
{
    char *ptr, *op, *left, *right;
    char string[STRINGSIZE];
    int truth;
    double result;
    LOGICAL error;
    COMMAND *comm;

    /*
     *  First, test if an open brace is present before anything
     *  else on the line.
     */
    if ((ptr = wipleading(*rest)) == (char *)NULL)    /* Nothing here. */
      return((COMMAND *)NULL);
    if (BRACE(*ptr)) {             /* Is this character an open brace? */
      right = wipbracextract(ptr, &left);
      if ((ptr != left) || (right == (char *)NULL)) {
        wipoutput(stderr, "IF command format error: %s\n", ptr);
        return((COMMAND *)NULL);
      }
      left++;                             /* Skip past the open brace. */
      *right++ = Null;     /* Remove closing brace and end the string. */
      *rest = right;                /* Reset rest to after expression. */
      ptr = left;                 /* ptr now points to the expression. */
    } else {                                  /* Input is just X op Y. */
      if ((left  = wipparse(rest)) == (char *)NULL) return((COMMAND *)NULL);
      if ((op    = wipparse(rest)) == (char *)NULL) return((COMMAND *)NULL);
      if ((right = wipparse(rest)) == (char *)NULL) return((COMMAND *)NULL);
      SPrintf(string, "%s %s %s", left, op, right);
      ptr = string;               /* ptr now points to the expression. */
    }
    /*
     *  Now, evaluate the expression.  The result should be 1 if
     *  the expression is true; 0 otherwise.
     */
    result = wipevaluate(ptr, &error);
    if (error == TRUE) {
      wipoutput(stderr, "Unrecognized conditional expression: %s\n", ptr);
      return((COMMAND *)NULL);
    }
    truth = NINT(result);

    /*  If the conditional fails, return the command ENDIF. */
    /*******************************************************************/
    /** Calling program needs to check for ENDIF in return statement. **/
    /*******************************************************************/
    if (!truth) {
      return(ENDIF);
    }

    /* Finally, check that macro name exists. */

    if ((comm = wipinterpret(rest)) == (COMMAND *)NULL)
      return((COMMAND *)NULL);

    /* Certain commands are not allowed; check for them. */

    if (Strcmp("define", comm->name) == 0) {
      wipoutput(stderr, "IF does not allow DEFINE.\n");
      return((COMMAND *)NULL);
    }

    if (Strcmp("insert", comm->name) == 0) {
      wipoutput(stderr, "IF does not allow INSERT.\n");
      return((COMMAND *)NULL);
    }

    if (Strcmp("input", comm->name) == 0) {
      wipoutput(stderr, "IF does not allow INPUT.\n");
      return((COMMAND *)NULL);
    }

    if (Strcmp("playback", comm->name) == 0) {
      wipoutput(stderr, "IF does not allow PLAYBACK.\n");
      return((COMMAND *)NULL);
    }

    return(comm);
}
