/*
	<list.c>
	31jul90 jm  Original code.
	07jun91 jm  Changed arg order from "list mac L1 L2" to "list L1 L2 mac"
        02aug92 jm  Modified to return status (void -> int) rather than
		    using a passed LOGICAL pointer.
        12dec94 jm  Modified call sequence to parse line externally.
         9oct00 pjt no more PROTOTYPEs


Routines:
int wiplist ARGS(( int line1, int line2, Const char *rest ));
*/

#define WIP_VOCAB
#include "wip.h"

/* Global variables for just this file */

/* Code */

/*  Returns 0 if successful; 1 on error. */
int wiplist(int line1, int line2, Const char *rest)
{
    char *par, *ptr;
    char string[STRINGSIZE];
    register int j;
    COMMAND *vb;
    PCMACRO *pc;

    vb = BUFFER;
    ptr = Strcpy(string, rest);
    if ((par = wipparse(&ptr)) != (char *)NULL) {
      if ((vb = find_command(BUFFER, par, 0)) == (COMMAND *)EOF) {
        wipoutput(stderr, "LIST:  Cannot find macro [%s].\n", par);
        return(1);
      }
    }

    if ((vb == (COMMAND *)NULL) || (vb->macro != TRUE)) {
      wipoutput(stderr, "LIST:  List `macro name'.\n");
      return(1);
    }

    if (line2 < 0) line2 = vb->ncom;                  /* Special case. */
    line1--;
    line1 = MIN(MAX(line1, 0), vb->ncom);
    line2 = MIN(MAX(line2, line1), vb->ncom) + 1;

    for (j = 1, pc = vb->pcmac; pc != (PCMACRO *)NULL; j++, pc = pc->next) {
      if ((j > line1) && (j < line2))
        wipoutput(stdout, " %4d %s\n", j, pc->line);
    }

    return(0);
}
