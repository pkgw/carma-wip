/*
	<interpret.c>
	18jul90 jm  Original code.
	16aug92 jm  Cleaned up and documented code a bit.
	12jan93 jm  Added exact match code section to find_command().
	14feb95 jm  Added exact argument to find_command().
	16jun95 jm  Fixed a bug that kept only 3 choices on a line.
         9oct00 pjt no more PROTOTYPEs


Routines:
COMMAND *find_command ARGS(( COMMAND *first, Const char *inname, int exact ));
COMMAND *wipinterpret ARGS(( char **line ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

/*
 *  The string "inname" points to a LOWER CASE name to be searched
 *  (using minimum match) for the corresponding command.  The search
 *  begins with the COMMAND structure pointed to by "first".  If
 *  the command is found (a match), a pointer to that structure is
 *  returned.  If no match, a pointer to EOF (see <stdio.h>) cast
 *  to a COMMAND pointer is returned.  A pointer to NULL is returned
 *  on any error.
 */
COMMAND *find_command(COMMAND *first, Const char *inname, int exact)
{
    char *ptr;
    Const char *endstr = "end";
    int numberOfChoices;
    size_t lenpar;
    COMMAND *ret;
    register COMMAND *vb;
    LOGICAL err, match;

    if ((ptr = wipleading(inname)) == (char *)NULL)
      return((COMMAND *)NULL);                   /* Nothing to search. */

    if ((Strcmp(ptr, "exit") == 0) || (Strcmp(ptr, "quit") == 0))
      ptr = (char *)endstr;              /* Special name substitution. */

    if ((lenpar = Strlen(ptr)) <= 0)             /* Nothing to search. */
      return((COMMAND *)NULL);

    /*  First, try to find an exact name match to the string "inname". */

    for (vb = first; vb != (COMMAND *)NULL; vb = vb->next) {
      if (Strcmp(ptr, vb->name) == 0)               /* An exact match. */
        return((COMMAND *)vb);
    }

    if (exact > 0)    /* If only exact matches are acceptable, return. */
      return((COMMAND *)EOF);

    /*  No exact match; try to find a minimum match to the name. */

    numberOfChoices = 0;
    err = match = FALSE;
    for (vb = first; vb != (COMMAND *)NULL; vb = vb->next) {
      if (Strncmp(ptr, vb->name, lenpar) == 0) {           /* A match. */
        if (match != TRUE) {                     /* No previous match. */
          ret = vb;
          match = TRUE;
        } else {           /* There was a previous match to this name. */
          if (err != TRUE) {
            wipoutput(stderr, "Ambiguous command name, choices are:\n");
            wipoutput(stderr, "%-20s", ret->name);
            numberOfChoices = 1;
            err = TRUE;
          }
          numberOfChoices++;
          if (numberOfChoices > 3) {   /* Four command names per line. */
            wipoutput(stderr, "%-19s\n", vb->name);
            numberOfChoices = 0;
          } else {
            wipoutput(stderr, "%-20s", vb->name);
          }
        }
      }
    }

    if (numberOfChoices > 0)
      wipoutput(stderr, "\n");

    if (match == FALSE) return((COMMAND *)EOF);
    if (err == TRUE) return((COMMAND *)NULL);
    return(ret);
}

/*
 *  Parses the input string and searches for the command.  If found,
 *  the command structure is returned; a pointer to NULL means either
 *  no command found or some other error.
 */
COMMAND *wipinterpret(char **line)
{
    char *par;
    COMMAND *vb;

    if ((par = wipparse(line)) == (char *)NULL)       /* Nothing here. */
      return((COMMAND *)NULL);
    wiplower(par);                /* Make the command name lower case. */

    if ((vb = find_command(HEAD, par, 0)) == (COMMAND *)EOF) {
      wipoutput(stderr, "Unknown command [%s].\n", par);
      return((COMMAND *)NULL);
    }

    return(vb);
}
